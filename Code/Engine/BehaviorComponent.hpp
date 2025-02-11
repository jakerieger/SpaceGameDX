#pragma once

#include "EngineCommon.hpp"
#include "Common/Types.hpp"
#include "Common/Filesystem.hpp"

namespace x {
    class BehaviorComponent {
    public:
        BehaviorComponent() = default;

        void LoadFromFile(const str& filename) {
            using namespace Filesystem;
            auto scriptSource = FileReader::ReadAllText(Path(filename));

            if (!scriptSource.empty()) {
                _scriptSource = std::move(scriptSource);
                _scriptPath   = filename;
                _scriptId     = Path(filename).Filename();
            }
        }

        void Reload() {
            if (!_scriptPath.empty()) {
                LoadFromFile(_scriptPath);
            }
        }

        X_NODISCARD const str& GetSource() const {
            return _scriptSource;
        }

        X_NODISCARD const str& GetId() const {
            return _scriptId;
        }

    private:
        str _scriptSource;
        str _scriptPath;
        str _scriptId;
    };
}