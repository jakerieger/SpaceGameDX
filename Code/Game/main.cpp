#include "Engine/Game.hpp"
#include "Engine/Model.hpp"
#include "Engine/GenericLoader.hpp"
#include "Engine/Material.hpp"
#include "Engine/RasterizerState.hpp"
#include "Engine/Texture.hpp"
#include "Common/Str.hpp"
#include "Common/Timer.hpp"
#include "Engine/TonemapEffect.hpp"
#include "Engine/ColorGradeEffect.hpp"
#include "imgui/imgui_internal.h"

using namespace x;

struct Vertex {
    XMFLOAT3 position;
    XMFLOAT4 color;
};

static str ContentPath(const str& filename) {
    const str root = R"(C:\Users\conta\Code\SpaceGame\Engine\Content\)";
    return root + filename;
}

class SpaceGame final : public IGame {
    shared_ptr<PBRMaterial> _monkeMaterial;
    shared_ptr<PBRMaterial> _floorMaterial;
    ModelHandle _monkeModel;
    ModelHandle _floorModel;
    f32 _monkeRotationY = 0.f;
    Matrix _monkeModelMatrix;
    Matrix _floorModelMatrix;
    TonemapEffect* _tonemap       = None;
    ColorGradeEffect* _colorGrade = None;
    f32 _contrast                 = 1.0f;
    f32 _saturation               = 1.0f;
    f32 _temperature              = 6500.0f;
    TonemapOperator _tonemapOp    = TonemapOperator::ACES;
    f32 _tonemapExposure          = 1.0f;
    bool _showPostProcessUI       = false;

public:
    explicit SpaceGame(const HINSTANCE instance) : IGame(instance, "SpaceGame", 1280, 720) {
        _monkeModelMatrix = XMMatrixIdentity();

        _floorModelMatrix = XMMatrixTranslation(0.0f, -1.0f, 0.0f); // move floor down below suzan
        // _floorModelMatrix = XMMatrixMultiply(_floorModelMatrix, XMMatrixScaling(100.0f, 100.0f, 100.0f));
    }

    void LoadContent(GameState& state) override {
        devConsole.RegisterCommand("r_ShowPostProcess",
                                   [this](auto args) {
                                       if (args.size() < 1) { return; }
                                       const auto show    = CAST<int>(strtol(args[0].c_str(), None, 10));
                                       _showPostProcessUI = show;
                                   });

        RasterizerStates::SetupRasterizerStates(renderer);

        GenericLoader loader(renderer);
        const auto monkeData = loader.LoadFromFile(ContentPath("Monke.glb"));
        _monkeModel.SetModelData(monkeData);

        const auto floorData = loader.LoadFromFile(ContentPath("Floor.glb"));
        _floorModel.SetModelData(floorData);

        if (!_monkeModel.Valid()) {
            PANIC("Failed to load model data.");
        }

        TextureLoader texLoader(renderer);

        const auto monkeAlbedo    = texLoader.LoadFromFile2D(ContentPath("Metal_Albedo.dds"));
        const auto monkeNormal    = texLoader.LoadFromFile2D(ContentPath("Metal_Normal.dds"));
        const auto monkeMetallic  = texLoader.LoadFromFile2D(ContentPath("Metal_Metallic.dds"));
        const auto monkeRoughness = texLoader.LoadFromFile2D(ContentPath("Metal_Roughness.dds"));

        const auto floorAlbedo = texLoader.LoadFromFile2D(ContentPath("checkerboard.dds"));
        const auto floorNormal = texLoader.LoadFromFile2D(ContentPath("Gold_Normal.dds"));
        // const auto floorMetallic  = texLoader.LoadFromFile2D(ContentPath("Gold_Metallic.dds"));
        // const auto floorRoughness = texLoader.LoadFromFile2D(ContentPath("Gold_Roughness.dds"));

        _monkeMaterial = PBRMaterial::Create(renderer);
        _monkeMaterial->SetTextureMaps(monkeAlbedo, monkeMetallic, monkeRoughness, monkeNormal);

        _floorMaterial = PBRMaterial::Create(renderer);
        _floorMaterial->SetTextureMaps(floorAlbedo, None, None, floorNormal);

        auto& camera = state.GetMainCamera();
        camera.SetFOV(70.0f);
        camera.SetPosition(XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f));

        auto& sun     = state.GetLightState().Sun;
        sun.enabled   = true;
        sun.intensity = 2.0f;
        sun.color     = {1.0f, 1.0f, 1.0f, 1.0f};
        sun.direction = {-0.57f, 0.37f, 0.97f, 1.0f};

        auto& pointLight0     = state.GetLightState().PointLights[0];
        pointLight0.enabled   = true;
        pointLight0.intensity = 20.0f;
        pointLight0.color     = {1.0f, 0.0f, 0.0f};
        pointLight0.position  = {5.0f, 3.0f, 0.0f};

        auto& pointLight1     = state.GetLightState().PointLights[1];
        pointLight1.enabled   = false;
        pointLight1.intensity = 20.0f;
        pointLight1.color     = {0.0f, 1.0f, 0.0f};
        pointLight1.position  = {-5.0f, 3.0f, 0.0f};

        auto& pointLight2     = state.GetLightState().PointLights[2];
        pointLight2.enabled   = false;
        pointLight2.intensity = 20.0f;
        pointLight2.color     = {0.0f, 0.0f, 1.0f};
        pointLight2.position  = {0.0f, 3.0f, 0.0f};

        // auto& areaLight0      = state.GetLightState().AreaLights[0];
        // areaLight0.enabled    = true;
        // areaLight0.intensity  = 1.0f;
        // areaLight0.color      = {1.0f, 0.0f, 1.0f};
        // areaLight0.dimensions = {10.f, 10.f};
        // areaLight0.position   = {0.0f, 0.0f, -5.0f};
        // areaLight0.direction  = {0.0f, 0.0f, 5.0f};

        renderer.GetContext()->RSSetState(RasterizerStates::DefaultSolid.Get());

        PostProcessSystem* postProcess = renderer.GetPostProcess();
        _tonemap                       = postProcess->AddEffect<TonemapEffect>();
        _tonemap->SetOperator(_tonemapOp);
        _tonemap->SetExposure(_tonemapExposure);

        _colorGrade = postProcess->AddEffect<ColorGradeEffect>();
        _colorGrade->SetContrast(_contrast);
        _colorGrade->SetSaturation(_saturation);
        _colorGrade->SetTemperature(_temperature);
    }

    void UnloadContent() override {}

    void Update(GameState& state, const Clock& clock) override {
        _monkeRotationY += CAST<f32>(clock.GetDeltaTime());
        _monkeModelMatrix = XMMatrixRotationY(_monkeRotationY);
    }

    void Render(const GameState& state) override {
        auto view = state.GetMainCamera().GetViewMatrix();
        auto proj = state.GetMainCamera().GetProjectionMatrix();

        TransformMatrices transformMatrices(_floorModelMatrix, view, proj);
        _floorMaterial->Apply(transformMatrices, state.GetLightState(), state.GetMainCamera().GetPosition());
        _floorModel.Draw();
        _floorMaterial->Clear();

        transformMatrices = TransformMatrices(_monkeModelMatrix, view, proj);
        _monkeMaterial->Apply(transformMatrices, state.GetLightState(), state.GetMainCamera().GetPosition());
        _monkeModel.Draw();
        _monkeMaterial->Clear();
    }

    void DrawDebugUI() override {
        static constexpr std::array<const char*, 4> tonemapOpNames = {"ACES", "Reinhard", "Filmic", "Linear"};
        static bool dropdownValueChanged                           = false;

        if (_showPostProcessUI) {
            ImGui::Begin("Post Processing");

            ImGui::SliderFloat("Contrast", &_contrast, 0.0f, 2.0f);
            ImGui::SliderFloat("Saturation", &_saturation, 0.0f, 2.0f);
            ImGui::SliderFloat("Temperature", &_temperature, 1000.0f, 10000.0f);
            ImGui::Separator();
            ImGui::SliderFloat("Exposure", &_tonemapExposure, 0.0f, 2.0f);

            if (ImGui::BeginCombo("Tonemap Operator", tonemapOpNames[(u32)_tonemapOp])) {
                for (size_t i = 0; i < tonemapOpNames.size(); i++) {
                    auto opName           = tonemapOpNames[i];
                    const bool isSelected = (_tonemapOp == CAST<TonemapOperator>(i));

                    if (ImGui::Selectable(opName, isSelected)) {
                        _tonemapOp           = CAST<TonemapOperator>(i);
                        dropdownValueChanged = true;
                    }

                    if (isSelected) { ImGui::SetItemDefaultFocus(); }
                }
                ImGui::EndCombo();
            }

            ImGui::End();

            _colorGrade->SetContrast(_contrast);
            _colorGrade->SetSaturation(_saturation);
            _colorGrade->SetTemperature(_temperature);
            _tonemap->SetExposure(_tonemapExposure);
            _tonemap->SetOperator(_tonemapOp);
        }
    }

    void OnResize(u32 width, u32 height) override {}
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
    SpaceGame game(hInstance);

    #ifndef NDEBUG
    game.EnableConsole();
    #endif

    game.EnableDebugUI();
    game.Run();

    return 0; // I know you don't have to, but I like the explicit nature of this.
}