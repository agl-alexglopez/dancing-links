#include "GUI/MiniGUI.h"
#include <memory>
#include <string>

namespace {

    class PokemonGUI: public ProblemHandler {
    public:
        PokemonGUI(GWindow& window);

    protected:
        void repaint() override;

    private:
    };

    PokemonGUI::PokemonGUI(GWindow& window) : ProblemHandler(window) {
    }

    void PokemonGUI::repaint() {
    }

}
GRAPHICS_HANDLER("Pokemon Planning", GWindow& window) {
    return std::make_shared<PokemonGUI>(window);
}
