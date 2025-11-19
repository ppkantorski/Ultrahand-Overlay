#define TESLA_INIT_IMPL
#include "gui_main.hpp"

class OverlaySysmodules : public tsl::Overlay {
  public:
    OverlaySysmodules() {}
    ~OverlaySysmodules() {}

    void initServices() override {
        pmshellInitialize();
    }

    void exitServices() override {
        pmshellExit();
    }

    std::unique_ptr<tsl::Gui> loadInitialGui() override {
        return std::make_unique<GuiMain>();
    }
};

int main(int argc, char** argv) {
    return tsl::loop<OverlaySysmodules>(argc, argv);
}