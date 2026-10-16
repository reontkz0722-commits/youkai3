// 切り分け用の最小プラグイン(機能なし)。メニューを開けるだけ。
// 使い方: この中身を、リポジトリの main.cpp と差し替えてビルドする。
#include <3ds.h>
#include "csvc.h"
#include <CTRPluginFramework.hpp>

namespace CTRPluginFramework
{
    void PatchProcess(FwkSettings &settings)
    {
    }

    void OnProcessExit(void)
    {
    }

    int main(void)
    {
        PluginMenu *menu = new PluginMenu("YW2 minimal test", 0, 1, 0, "Minimal plugin for testing.");
        menu->SynchronizeWithFrame(true);
        menu->Run();
        delete menu;
        return 0;
    }
}
