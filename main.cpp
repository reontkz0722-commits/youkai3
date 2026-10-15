// 妖怪ウォッチ3 スキヤキ マップワープ (CTRPluginFramework)
// ※未ビルド・未テスト。Nanquitas の CTRPluginFrameworkBlankTemplate の Sources/main.cpp と差し替えて使う。
//
// 動作: メニューで行き先とキャラを選び、ホットキーを押している間、
//       次マップIDのアドレスへ行き先IDを書き込む(ARコードと同じ動き)。
// アドレス(New3DS用・wikiのNEW用ARコード): オフセット指定なしの絶対アドレス
//   ケータ : 0x0C3266E4
//   イナホ : 0x0C3268F4
// ARコードは2ブロック構成で、A+B中は両方を毎フレーム実行する:
//   1) F0000001 00000001 / D3000000 0C4F09C0 / DD000000 00000003 / F1000000 DD000000
//      = 0C4F09C0 のfloat値に 0xDD000000 (約 -5.8e17) を足す(移動を起こすきっかけ)
//   2) 現在のマップID(0C3266E4 など)を行き先IDで上書きする

#include <3ds.h>
#include "csvc.h"
#include <CTRPluginFramework.hpp>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace CTRPluginFramework
{
    struct MapInfo
    {
        const char *name;
        u32         id;
    };

    // 名前の5文字目(index 4)が種別: g=フィールド i=屋内 d=ダンジョン s=駅 それ以外=その他
    static const MapInfo g_maps[] =
    {
        {"Uptown - Triangle Way", 0xBF8FEA80},
        {"e100g002", 0xC36D2D22},
        {"e101g003", 0x890A3404},
        {"e101g005", 0x60699131},
        {"e104d331", 0x943A982C},
        {"e200g001", 0x6B8C6605},
        {"e200g002", 0xF28537BF},
        {"e200g003", 0x85820729},
        {"e200g004", 0x1BE6928A},
        {"e200g011", 0x72975744},
        {"e200g012", 0xEB9E06FE},
        {"e200g013", 0x9C993668},
        {"e200g014", 0x02FDA3CB},
        {"e200g015", 0x75FA935D},
        {"e200g016", 0xECF3C2E7},
        {"e200g017", 0x9BF4F271},
        {"e200g018", 0x0B4BEFE0},
        {"e200g019", 0x7C4CDF76},
        {"e201g001", 0x56EC4FB5},
        {"e201g002", 0xCFE51E0F},
        {"e201g003", 0xB8E22E99},
        {"e201g005", 0x51818BAC},
        {"e201g006", 0xC888DA16},
        {"e201g009", 0x5837C787},
        {"e202g001", 0x114C3565},
        {"e203g001", 0x2C2C1CD5},
        {"e203g003", 0xC2227DF9},
        {"e203g005", 0x2B41D8CC},
        {"e203g007", 0xC54FB9E0},
        {"e205g001", 0xA36CE975},
        {"e205g003", 0x4D628859},
        {"e205g005", 0xA4012D6C},
        {"e206g001", 0xE4CC93A5},
        {"e206g003", 0x0AC2F289},
        {"e210g001", 0xA0D0B5A0},
        {"e210g003", 0x4EDED48C},
        {"e210g005", 0xA7BD71B9},
        {"e210g011", 0xB9CB84E1},
        {"e210g012", 0x20C2D55B},
        {"e250g001", 0x3B41F7B6},
        {"e260g002", 0x24DCD4A2},
        {"e260g003", 0x53DBE434},
        {"e260g004", 0xCDBF7197},
        {"e260g005", 0xBAB84101},
        {"e260g006", 0x23B110BB},
        {"e260g007", 0x54B6202D},
        {"e260g008", 0xC4093DBC},
        {"e260g009", 0xB30E0D2A},
        {"e260g010", 0xD3C984CF},
        {"e270g001", 0x768956BD},
        {"e270g003", 0x98873791},
        {"e270g005", 0x71E492A4},
        {"e270g007", 0x9FEAF388},
        {"e300g001", 0xCDFB6DB1},
        {"e300g002", 0x54F23C0B},
        {"e300g003", 0x23F50C9D},
        {"e300g004", 0xBD91993E},
        {"e300g006", 0x539FF812},
        {"e300g011", 0xD4E05CF0},
        {"e300g012", 0x4DE90D4A},
        {"e300g013", 0x3AEE3DDC},
        {"e300g014", 0xA48AA87F},
        {"e300g920", 0x871B042A},
        {"e301g011", 0xE9807540},
        {"e301g012", 0x708924FA},
        {"e301g013", 0x078E146C},
        {"e301g021", 0xC2AD2683},
        {"e301g022", 0x5BA47739},
        {"e301g023", 0x2CA347AF},
        {"e301g030", 0xACB12754},
        {"e301g150", 0xFB29EAE5},
        {"e301g180", 0x4E8794A8},
        {"e301g280", 0x4CC12AF1},
        {"e301g310", 0x9CC1FB8F},
        {"e301g990", 0x598FF451},
        {"e302g060", 0x9666A9C1},
        {"e302g720", 0xF7457A40},
        {"e302g950", 0xB29AC18D},
        {"e303g230", 0xD5F5A05A},
        {"e303g250", 0x83AF07DC},
        {"e303g430", 0xD178DCE8},
        {"e303g630", 0xD2FC0886},
        {"e303g860", 0xA515D1C9},
        {"e304g200", 0x4CF82F89},
        {"e304g510", 0x50AC084D},
        {"e305g480", 0xBDCCF083},
        {"e305g482", 0x53C291AF},
        {"e305g910", 0x64D6D899},
        {"e305g950", 0x00BA1D9D},
        {"e306g020", 0x078ACA05},
        {"e306g050", 0x48CB5CC2},
        {"e306g300", 0x37FA16DE},
        {"e306g610", 0x282AE574},
        {"e307g660", 0x5A0B5A03},
        {"e309g170", 0xF96FC3A6},
        {"e309g780", 0x7A7AA3DB},
        {"e310g150", 0x0D1510F0},
        {"e310g650", 0x085A0675},
        {"e310g660", 0x237755B6},
        {"e310g670", 0x3A6C64F7},
        {"e310g710", 0x6DF4A946},
        {"e311g500", 0x4A0B65D9},
        {"e312g080 Mysterious Ruins", 0xC3B957EA},
        {"e360g001", 0x1BA28EAC},
        {"e360g002", 0x82ABDF16},
        {"e360g003", 0xF5ACEF80},
        {"e360g004", 0x6BC87A23},
        {"e360g005", 0x1CCF4AB5},
        {"e360g006", 0x85C61B0F},
        {"e360g007", 0xF2C12B99},
        {"e360g008", 0x627E3608},
        {"e360g009", 0x1579069E},
        {"e360g010", 0x75BE8F7B},
        {"e360g011", 0x02B9BFED},
        {"e360g012", 0x9BB0EE57},
        {"e360g013", 0xECB7DEC1},
        {"e360g014", 0x72D34B62},
        {"e360g017", 0xEBDA1AD8},
        {"e360g018", 0x7B650749},
        {"e360g019", 0x0C6237DF},
        {"e360g360", 0x38B9A7E5},
        {"e370g010", 0xBEE25CDE},
        {"e370g310", 0xBCA4E287},
        {"t001b01", 0xAD4C222E},
        {"t001b02", 0x34457394},
        {"t001b03", 0x43424302},
        {"t001b11", 0xB457136F},
        {"t001b12", 0x2D5E42D5},
        {"t001b13", 0x5A597243},
        {"t001b15", 0xB33AD776},
        {"t001b21", 0x9F7A40AC},
        {"t001b23", 0x71742180},
        {"t001b51", 0xD03BD66B},
        {"t001b53", 0x3E35B747},
        {"t001b55", 0xD7561272},
        {"t001b57", 0x3958735E},
        {"t001b59", 0xDEE05E59},
        {"t001b61", 0xFB1685A8},
        {"t001b71", 0xE20DB4E9},
        {"t001b73", 0x0C03D5C5},
        {"t001b75", 0xE56070F0},
        {"t001b76", 0x7C69214A},
        {"t001b77", 0x0B6E11DC},
        {"t001b78", 0x9BD10C4D},
        {"t001b79", 0xECD63CDB},
        {"t001b80", 0x129298B0},
        {"t001b81", 0x6595A826},
        {"t001d21 Yopple HQ - Garden", 0x9BF73C1E},
        {"t001d23 Yopple HQ - Lobby", 0x75F95D32},
        {"t001d25 Yopple HQ - CEO's Office", 0x9C9AF807},
        {"t001d27 Yopple HQ - Yo-kai Watch Center", 0x7294992B},
        {"t001d29 Yopple HQ - Meeting Room", 0x952CB42C},
        {"t001d31 Yopple HQ - Lab", 0x82EC0D5F},
        {"t001d33 Yopple HQ - Assembly Line", 0x6CE26C73},
        {"t001d35 Yopple HQ - Fighting Room", 0x8581C946},
        {"t001d37 Yopple HQ - Cafeteria", 0x6B8FA86A},
        {"t001d39 Yopple HQ - The Fish Tank", 0x8C37856D},
        {"t001d41 Ambrosia Pavilion - Entrance", 0xCDAD9B98},
        {"t001d42 Yo-kai World - Entry Hall", 0x54A4CA22},
        {"t001d46 Ambrosia Pavilion", 0x53C90E3B},
        {"t001d49 Treasure Ship of the Seven Gods", 0xC37613AA},
        {"t001d51 Bada-Bing Tower - Lobby", 0xD4B6AAD9},
        {"t001d53 Bada-Bing Tower - Elevator", 0x3AB8CBF5},
        {"t001d55 Dangerous Floor", 0xD3DB6EC0},
        {"t001d57 Dream Floor - Moon", 0x3DD50FEC},
        {"t001d59 Dream Floor - Fire", 0xDA6D22EB},
        {"t001d61 Dream Floor - Water", 0xFF9BF91A},
        {"t001d63 Dream Floor - Woods", 0x11959836},
        {"t001d64 Dream Floor - Woods", 0x8FF10D95},
        {"t001d65 Dream Floor - Gold", 0xF8F63D03},
        {"t001d66 Dream Floor - Gold", 0x61FF6CB9},
        {"t001d67 Dream Floor - Earth", 0x16F85C2F},
        {"t001d69 Dream Floor - Sun", 0xF1407128},
        {"t001d71 Ghoulfather's Office", 0xE680C85B},
        {"t001d73 UFO Interior", 0x088EA977},
        {"t001g00 New Yo-kai City", 0xDC80D053},
        {"t001i51 Club Koma", 0xDC6E398A},
        {"t001i53 Enma Villa", 0x326058A6},
        {"t001i55 Yopple Store", 0xDB03FD93},
        {"t001i57 Phoenix Sushi", 0x350D9CBF},
        {"t001i59 Serenity Tempura", 0xD2B5B1B8},
        {"t001i60 Blasters House - B1", 0x80445ADF},
        {"t001i61 Blasters House", 0xF7436A49},
        {"t001i71 Haven Sukiyaki", 0xEE585B08},
        {"t001t01", 0xB5E7FDEC},
        {"t001t03", 0x5BE99CC0},
        {"t001t04", 0xC58D0963},
        {"t001w011", 0x386009F7},
        {"t002b01", 0xBFF98DC0},
        {"t002t03", 0x495C332E},
        {"t002t04", 0xD738A68D},
        {"t100b01", 0xDEAC96EE},
        {"t100b02", 0x47A5C754},
        {"t100b02a", 0x84F68E69},
        {"t100b02b", 0x1DFFDFD3},
        {"t100b03", 0x30A2F7C2},
        {"t100b04", 0xAEC66261},
        {"t100b05", 0xD9C152F7},
        {"t100b06", 0x40C8034D},
        {"t100b07", 0x37CF33DB},
        {"t100b21", 0xEC9AF46C},
        {"t100b21a", 0xAC5F09C4},
        {"t100b21b", 0x3556587E},
        {"t100b22", 0x7593A5D6},
        {"t100b22c", 0x697C3B2B},
        {"t100b23", 0x02949540},
        {"t100b23c", 0x70670A6A},
        {"t100b24", 0x9CF000E3},
        {"t100b24c", 0x3F269CAD},
        {"t100b27", 0x05F95159},
        {"t100b29", 0xE2417C5E},
        {"t100b41", 0xBAC053EA},
        {"t100b51", 0xA3DB62AB},
        {"t100b52", 0x3AD23311},
        {"t100b53", 0x4DD50387},
        {"t100b55", 0xA4B6A6B2},
        {"t100b57", 0x4AB8C79E},
        {"t100b59", 0xAD00EA99},
        {"t100b61", 0x88F63168},
        {"t100b62", 0x11FF60D2},
        {"t100b63", 0x66F85044},
        {"t100b64", 0xF89CC5E7},
        {"t100b65", 0x8F9BF571},
        {"t100b66", 0x1692A4CB},
        {"t100b67", 0x6195945D},
        {"t100b68", 0xF12A89CC},
        {"t100b69", 0x862DB95A},
        {"t100b70", 0xE6EA30BF},
        {"t100b71", 0x91ED0029},
        {"t100b72", 0x08E45193},
        {"t100b73", 0x7FE36105},
        {"t100b74", 0xE187F4A6},
        {"t100b75", 0x9680C430},
        {"t100b76", 0x0F89958A},
        {"t100b77", 0x788EA51C},
        {"t100b78", 0xE831B88D},
        {"t100b79", 0x9F36881B},
        {"t100b80", 0x61722C70},
        {"t100b81", 0x16751CE6},
        {"t100b82", 0x8F7C4D5C},
        {"t100d00", 0xAD26DACA},
        {"t100d01 Springdale Sewers - Mouse Alley", 0xDA21EA5C},
        {"t100d02 Springdale Sewers - Secret Space", 0x4328BBE6},
        {"t100d03 Springdale Sewers - Cat Square", 0x342F8B70},
        {"t100d04 Springdale Sewers - Neon Town", 0xAA4B1ED3},
        {"t100d05 Springdale Sewers - Main Street", 0xDD4C2E45},
        {"t100d06 Springdale Sewers - School Route", 0x44457FFF},
        {"t100d07 Springdale Sewers - Kaos March Mural", 0x33424F69},
        {"t100d08 Springdale Sewers - Bracing Path", 0xA3FD52F8},
        {"t100d09 Springdale Sewers - Breezy Hills Byway", 0xD4FA626E},
        {"t100d10 Springdale Sewers - Frogetmenot's Inn", 0xB43DEB8B},
        {"t100d11 Sparkopolis Sewers - Meeting Place", 0xC33ADB1D},
        {"t100d12 Springdale Sewers - Yo-kai Parade Mural", 0x5A338AA7},
        {"t100d13 Springdale Sewers - Downtown Westside", 0x2D34BA31},
        {"t100i21 Mirror of Truth", 0xE0CF1B8D},
        {"t100i23 ?", 0x0EC17AA1},
        {"t100i25 ?", 0xE7A2DF94},
        {"t100s01 Springdale Central Station", 0xC3485FA9},
        {"t100s02 Green St Station (Sparkopolis)", 0x5A410E13},
        {"t100s03 Hibarly Hills Station", 0x2D463E85},
        {"t100s04 Petal Peak Station", 0xB322AB26},
        {"t100s05 Factory Row Station", 0xC4259BB0},
        {"t100s06 Sweet Meadow Station", 0x5D2CCA0A},
        {"t100s07 Fortune Place Station", 0x2A2BFA9C},
        {"t100s08 Skybridge Station", 0xBA94E70D},
        {"t100s09 Dreamer's Field Station", 0xCD93D79B},
        {"t100s10 Ridgemont Station", 0xAD545E7E},
        {"t100s11 Bayside Station", 0xDA536EE8},
        {"t100s12 San Fantastico Station", 0x435A3F52},
        {"t100s13 Greenfields Station", 0x345D0FC4},
        {"t100s14 Temple Park Station", 0xAA399A67},
        {"t100s15 Dingle Falls Station", 0xDD3EAAF1},
        {"t100s16 Harrisville Station", 0x4437FB4B},
        {"t100s17 Spring Station", 0x3330CBDD},
        {"t100s18 Sunshine Station", 0xA38FD64C},
        {"t100s19 Little Haven Station", 0xD488E6DA},
        {"t100s20 Scarfit Downs Station", 0x86790DBD},
        {"t100s21 Cherry Hill Station", 0xF17E3D2B},
        {"t100s22 Whimsy Valley Station", 0x68776C91},
        {"t100t00 Train Interior #1 (Default)", 0xB10079BA},
        {"t100t01 Train Interior #2", 0xC607492C},
        {"t100t02 Train Interior #3", 0x5F0E1896},
        {"t100t03 Train Interior #4", 0x28092800},
        {"t100t04 Train Interior #5", 0xB66DBDA3},
        {"t100t05 Train Interior #6", 0xC16A8D35},
        {"t100t06 Train Interior #7", 0x5863DC8F},
        {"t100w011", 0xA3772BF3},
        {"t100w021", 0x885A7830},
        {"t100w031", 0x91414971},
        {"t100w041", 0xDE00DFB6},
        {"t100w051", 0xC71BEEF7},
        {"t100w061", 0xEC36BD34},
        {"t100w071", 0xF52D8C75},
        {"t100w081", 0x72B590BA},
        {"t100w091", 0x6BAEA1FB},
        {"t100w101", 0xBBAE7085},
        {"t100w111", 0xA2B541C4},
        {"t100w121", 0x89981207},
        {"t100w131", 0x90832346},
        {"t100w141", 0xDFC2B581},
        {"t100w151", 0xC6D984C0},
        {"t100w161", 0xEDF4D703},
        {"t100w171", 0xF4EFE642},
        {"t100w181", 0x7377FA8D},
        {"t100w191", 0x6A6CCBCC},
        {"t100w201", 0xB9E8CEDC},
        {"t100w211", 0xA0F3FF9D},
        {"t101b01", 0x6610F18B},
        {"t101b02", 0xFF19A031},
        {"t101b03", 0x881E90A7},
        {"t101b21", 0x54269309},
        {"t101b23", 0xBA28F225},
        {"t101b25", 0x534B5710},
        {"t101b27", 0xBD45363C},
        {"t101b28", 0x2DFA2BAD},
        {"t101b31", 0x4D3DA248},
        {"t101b32", 0xD434F3F2},
        {"t101d01 Shady Back Alley", 0x629D8D39},
        {"t101d02 Lonely Waterway", 0xFB94DC83},
        {"t101d03 The Catwalk", 0x8C93EC15},
        {"t101d05 Desolate Lane", 0x65F04920},
        {"t101g00 Uptown Springdale", 0x17DC03F6},
        {"t101i01 Adams House - 1F", 0x6A451E6A},
        {"t101i02 Adams House - 2F", 0xF34C4FD0},
        {"t101i03 Forester House - 1F", 0x844B7F46},
        {"t101i04 Forester House - 2F", 0x1A2FEAE5},
        {"t101i10 Logan's House", 0x04591FBD},
        {"t101i21 Banter Bakery", 0x58737CE8},
        {"t101i23 Everymart Uptown", 0xB67D1DC4},
        {"t101i25 Springdale Community Center", 0x5F1EB8F1},
        {"t101i27 Piggleston Bank", 0xB110D9DD},
        {"t101i29 Lambert Post Office", 0x56A8F4DA},
        {"t101i31 Jungle Hunter", 0x41684DA9},
        {"t101i51 Springdale Elementary - 1F South", 0x1732EA2F},
        {"t101i52 Springdale Elementary - 1F North", 0x8E3BBB95},
        {"t101i53 Springdale Elementary - 2F", 0xF93C8B03},
        {"t101i55 Springdale Elementary - 3F", 0x105F2E36},
        {"t101i58 Springdale Elementary - Roof", 0x6EEE528B},
        {"t101i59 Springdale Elementary Gym", 0x19E9621D},
        {"t102b01", 0x74A55E65},
        {"t102b02", 0xEDAC0FDF},
        {"t102b03", 0x9AAB3F49},
        {"t102b11", 0x6DBE6F24},
        {"t102b21", 0x46933CE7},
        {"t102b23", 0xA89D5DCB},
        {"t102d01 Mt. Wildwood - Mountain Path", 0x702822D7},
        {"t102d02 Mt. Wildwood - Summit", 0xE921736D},
        {"t102d03 Jumbo Slider", 0x9E2643FB},
        {"t102d31 Abandoned Tunnel West", 0x5B057114},
        {"t102d32 Abandoned Tunnel East", 0xC20C20AE},
        {"t102g00 Mt. Wildwood", 0x0569AC18},
        {"t102i01 Deserted House", 0x78F0B184},
        {"t102i21 Shrine Behind the Waterfall", 0x4AC6D306},
        {"t102w011", 0xD9B77893},
        {"t103b01", 0xCC193900},
        {"t103b03", 0x2217582C},
        {"t103b05", 0xCB74FD19},
        {"t103b07", 0x257A9C35},
        {"t103b09", 0xC2C2B132},
        {"t103b21", 0xFE2F5B82},
        {"t103b23", 0x10213AAE},
        {"t103d01 Tucked Away Lot", 0xC89445B2},
        {"t103d03 Hidden Side Street", 0x269A249E},
        {"t103d11 Secret Byway", 0xD18F74F3},
        {"t103d31 Old Mansion - Main House", 0xE3B91671},
        {"t103d33 Old Mansion - Side House", 0x0DB7775D},
        {"t103d35 Old Mansion - Main House Attic", 0xE4D4D268},
        {"t103d36 Old Mansion - Side House Attic", 0x7DDD83D2},
        {"t103g00 Blossom Heights", 0xBDD5CB7D},
        {"t103i01 Bernstein House - 1F", 0xC04CD6E1},
        {"t103i02 Bernstein House - 2F", 0x5945875B},
        {"t103i03 Bernstein House - 3F", 0x2E42B7CD},
        {"t103i10 Megan's House", 0xAE50D736},
        {"t103i21 Timers & More", 0xF27AB463},
        {"t103i23 Candy Stop", 0x1C74D54F},
        {"t103i25 Everymart Blossom Heights", 0xF517707A},
        {"t103i31 Shoten Temple", 0xEB618522},
        {"t103i33 Prayer's Peak Tunnel", 0x056FE40E},
        {"t103i35 Chloro-Phil Good", 0xEC0C413B},
        {"t103i37 Springdale Hot Springs - Lobby", 0x02022017},
        {"t103i38 Springdale Hot Springs - Men", 0x92BD3D86},
        {"t103i39 Springdale Hot Springs - Women", 0xE5BA0D10},
        {"t103i51 Byrd House - Mayan Room", 0xBD3B22A4},
        {"t103i53 Byrd House - Hidden Room", 0x53354388},
        {"t103i60 Wayfarer Manor - Room 101", 0xE11141F1},
        {"t103i61 Wayfarer Manor - Room 102", 0x96167167},
        {"t103i62 Wayfarer Manor - Room 103", 0x0F1F20DD},
        {"t103i63 Wayfarer Manor - Room 104", 0x7818104B},
        {"t103i64 Wayfarer Manor - Room 105", 0xE67C85E8},
        {"t103i65 Wayfarer Manor - Room 201", 0x917BB57E},
        {"t103i66 Wayfarer Manor - Room 202", 0x0872E4C4},
        {"t103i67 Wayfarer Manor - Room 203", 0x7F75D452},
        {"t103i68 Wayfarer Manor - Room 204", 0xEFCAC9C3},
        {"t103i69 Wayfarer Manor - VIP Room", 0x98CDF955},
        {"t104b01", 0x51CE01B9},
        {"t104b03", 0xBFC06095},
        {"t104b21", 0x63F8633B},
        {"t104b23", 0x8DF60217},
        {"t104b25", 0x6495A722},
        {"t104b27", 0x8A9BC60E},
        {"t104b29", 0x6D23EB09},
        {"t104b31", 0x7AE3527A},
        {"t104b33", 0x94ED3356},
        {"t104b35", 0x7D8E9663},
        {"t104b37", 0x9380F74F},
        {"t104d01 Academy Shortcut", 0x55437D0B},
        {"t104d03 Behind Frostia's Place", 0xBB4D1C27},
        {"t104d05 Delivery Bay", 0x522EB912},
        {"t104d07 Springdale Underground Parking", 0xBC20D83E},
        {"t104d11 Springdale Business Tower - 4F", 0x4C584C4A},
        {"t104d13 Springdale Business Tower - 13F", 0xA2562D66},
        {"t104d15 Springdale Business Tower - East Stairs", 0x4B358853},
        {"t104d17 Springdale Business Tower - West Stairs", 0xA53BE97F},
        {"t104d21 Springdale Business Tower - CEO's Off.", 0x67751F89},
        {"t104g00 Downtown Springdale", 0x2002F3C4},
        {"t104i10 Springdale Underground Area", 0x3387EF8F},
        {"t104i21 Seabreeze Tunnel", 0x6FAD8CDA},
        {"t104i23 Frostia's Place", 0x81A3EDF6},
        {"t104i25 Arcadia Arcade", 0x68C048C3},
        {"t104i27 Nom Burger", 0x86CE29EF},
        {"t104i29 Fortune Hospital - 1F", 0x617604E8},
        {"t104i30 Fortune Hospital - 2F", 0x01B18D0D},
        {"t104i31 Foundation Academy", 0x76B6BD9B},
        {"t104i33 Everymart Downtown", 0x98B8DCB7},
        {"t104i35 Cafe Shanista", 0x71DB7982},
        {"t104i37 Springdale Sports Club - 1F", 0x9FD518AE},
        {"t104i38 Springdale Sports Club - 2F", 0x0F6A053F},
        {"t104i39 Springdale Sports Club - 3F", 0x786D35A9},
        {"t104i41 Belly Buster Curry House", 0x39F72B5C},
        {"t104i43 Tortoise Camera Shop", 0xD7F94A70},
        {"t104i45 Sunset Mall - B1", 0x3E9AEF45},
        {"t104i47 Sunset Mall - 1F", 0xD0948E69},
        {"t104i49 Sunset Mall - Dolphin Cafe", 0x372CA36E},
        {"t104i51 Springdale Business Tower - 1F", 0x20EC1A1D},
        {"t104i53 Springdale Business Tower - 7F", 0xCEE27B31},
        {"t104i55 Sumptuous Sukiyaki", 0x2781DE04},
        {"t104i61 Springdale Central Station", 0x0BC149DE},
        {"t105b01", 0xE97266DC},
        {"t105b03", 0x077C07F0},
        {"t105b21", 0xDB44045E},
        {"t105b23", 0x354A6572},
        {"t105b25", 0xDC29C047},
        {"t105d01 Shopping Street Narrows", 0xEDFF1A6E},
        {"t105d11 Tranquility Apts. - A-102", 0xF4E42B2F},
        {"t105d12 Tranquility Apts. - A-104", 0x6DED7A95},
        {"t105d13 Tranquility Apts. - A-201", 0x1AEA4A03},
        {"t105d14 Tranquility Apts. - A-303", 0x848EDFA0},
        {"t105d15 Tranquility Apts. - B-202", 0xF389EF36},
        {"t105d16 Tranquility Apts. - B-204", 0x6A80BE8C},
        {"t105d17 Tranquility Apts. - B-301", 0x1D878E1A},
        {"t105d18 Tranquility Apts. - B-302", 0x8D38938B},
        {"t105d19 Tranquility Apts. - C-102", 0xFA3FA31D},
        {"t105d20 Tranquility Apts. - B-102", 0xA8CE487A},
        {"t105d21 Tranquility Apts. - B-101", 0xDFC978EC},
        {"t105d22 Tranquility Apts. - C-104", 0x46C02956},
        {"t105d23 Tranquility Apts. - A-202", 0x31C719C0},
        {"t105d24 Tranquility Apts. - A-204", 0xAFA38C63},
        {"t105d25 Tranquility Apts. - A-103", 0xD8A4BCF5},
        {"t105d26 Tranquility Apts. - C-201", 0x41ADED4F},
        {"t105d27 Tranquility Apts. - A-302", 0x36AADDD9},
        {"t105d28 Tranquility Apts. - B-303", 0xA615C048},
        {"t105d29 Tranquility Apts. - C-203", 0xD112F0DE},
        {"t105d30 Tranquility Apts. - C-204", 0xB1D5793B},
        {"t105d31 Tranquility Apts. - C-301", 0xC6D249AD},
        {"t105d32 Tranquility Apts. - C-302", 0x5FDB1817},
        {"t105d33 Tranquility Apts. - B-301 (Past)", 0x28DC2881},
        {"t105d34 Tranquility Apts. - B-302 (Past)", 0xB6B8BD22},
        {"t105d35 Tranquility Apts. - B-303 (Past)", 0xC1BF8DB4},
        {"t105d36 Tranquility Apts. - B-304 (Past)", 0x58B6DC0E},
        {"t105d41 Nocturne Hospital - 1F", 0x8993DF6A},
        {"t105d43 Nocturne Hospital - 2F", 0x679DBE46},
        {"t105d45 Nocturne Hospital - 3F", 0x8EFE1B73},
        {"t105d47 Hospital - Basement", 0x60F07A5F},
        {"t105d48 Hospital - Basement Lab", 0xF04F67CE},
        {"t105g00 Shopper's Row", 0x98BE94A1},
        {"t105i20 Springdale Flower Road", 0xA016DB29},
        {"t105i21 Settle In Bookstore", 0xD711EBBF},
        {"t105i23 North Wind Ramen", 0x391F8A93},
        {"t105i25 Everymart Shopper's Row", 0xD07C2FA6},
        {"t105i27 Sun Pavilion", 0x3E724E8A},
        {"t105i29 Toys iZ We", 0xD9CA638D},
        {"t105i31 Mary's Coin Laundry", 0xCE0ADAFE},
        {"t105i33 Superior Style", 0x2004BBD2},
        {"t105i35 Whatta Find", 0xC9671EE7},
        {"t105i37 Sushi Springdale", 0x27697FCB},
        {"t105i39 Tempura Tempest", 0xC0D152CC},
        {"t106b01", 0xFBC7C932},
        {"t106b21", 0xC9F1ABB0},
        {"t106b23", 0x27FFCA9C},
        {"t106b31", 0xD0EA9AF1},
        {"t106b33", 0x3EE4FBDD},
        {"t106d11 Rugged Path", 0xE65184C1},
        {"t106d31 Gourd Pond Museum - 1F", 0xD467E643},
        {"t106d32 Gourd Pond Museum - 2F", 0x4D6EB7F9},
        {"t106d33 Gourd Pond Museum - Vault", 0x3A69876F},
        {"t106g00 Breezy Hills", 0x8A0B3B4F},
        {"t106i01 Archer House - 1F", 0xF79226D3},
        {"t106i02 Archer House - 2F", 0x6E9B7769},
        {"t106i05 Stone House", 0xF0FFE2CA},
        {"t106i07 Amy's House - 1F", 0x1EF183E6},
        {"t106i09 Amy's House - 2F", 0xF949AEE1},
        {"t106i10 Lina's House", 0x998E2704},
        {"t106i11 Thomas House", 0xEE891792},
        {"t106i13 Breezy Hills Apartments - Entrance", 0x008776BE},
        {"t106i15 Breezy Hills Apartments - 7F", 0xE9E4D38B},
        {"t106i21 Everymart Breezy Hills", 0xC5A44451},
        {"t106i23 Trophy Room", 0x2BAA257D},
        {"t106i51 Wisteria Gardens Parking", 0x8AE5D296},
        {"t106i52 Wisteria Gardens Entrance", 0x13EC832C},
        {"t106i55 Stonewood House", 0x8D88168F},
        {"t107b01", 0x437BAE57},
        {"t107b03", 0xAD75CF7B},
        {"t107g00 Excellent Tower", 0x32B75C2A},
        {"t107i01 Excellent Tower - Entrance", 0x4F2E41B6},
        {"t107i02 Excellent Tower - Elevator", 0xD627100C},
        {"t107i03 Excellent Tower - Observation Deck", 0xA120209A},
        {"t107w011", 0x1157F7E3},
        {"t107w021", 0x3A7AA420},
        {"t108b01", 0x1B18BE01},
        {"t108b03", 0xF516DF2D},
        {"t108b05", 0x1C757A18},
        {"t108b07", 0xF27B1B34},
        {"t108b09", 0x15C33633},
        {"t108d01 Shady Parking Lot", 0x1F95C2B3},
        {"t108d03 Junk Alley", 0xF19BA39F},
        {"t108g00 Sparkopolis", 0x6AD44C7C},
        {"t108i01 Next HarMEOWny Theater - Entrance", 0x174D51E0},
        {"t108i02 Next HarMEOWny Theater - Stage", 0x8E44005A},
        {"t108i05 AnimeChum", 0x102095F9},
        {"t108i07 Maid in Heaven", 0xFE2EF4D5},
        {"t108i09 Everymart Sparkopolis", 0x1996D9D2},
        {"t108i11 Detective Agency", 0x0E5660A1},
        {"t108i13 Soba Sensation", 0xE058018D},
        {"t109b01", 0xA3A4D964},
        {"t109b21", 0x9192BBE6},
        {"t109d11 Hazy Manor", 0xBE329497},
        {"t109d13 Hazy Manor", 0x503CF5BB},
        {"t109d15 Lotus Lake", 0xB95F508E},
        {"t109d17 Cape of Evening Calm", 0x575131A2},
        {"t109d19 Forevermore Falls", 0xB0E91CA5},
        {"t109d21 Classroom", 0x951FC754},
        {"t109d23 Blind Alley", 0x7B11A678},
        {"t109d25 Requiem Riverside", 0x9272034D},
        {"t109d30", 0xFB03C683},
        {"t109g00 Greenfields Station", 0xD2682B19},
        {"t121b01", 0x1CD0A2EB},
        {"t121b03", 0xF2DEC3C7},
        {"t121b03c", 0x034B56BF},
        {"t121b05", 0x1BBD66F2},
        {"t121b21", 0x2EE6C069},
        {"t121b23", 0xC0E8A145},
        {"t121b31", 0x37FDF128},
        {"t121d01 Briny Grotto", 0x185DDE59},
        {"t121d03 Hidden Workshop", 0xF653BF75},
        {"t121d11 Seaside Cave", 0x0146EF18},
        {"t121d13 Dragon Heights", 0xEF488E34},
        {"t121g00 San Fantastico", 0x6D1C5096},
        {"t121i01 Rolling Waves Meeting Hall", 0x10854D0A},
        {"t121i03 Rusty's Mart", 0xFE8B2C26},
        {"t121i10 Deserted House", 0x7E994CDD},
        {"t121i20 Sea", 0x55B41F1E},
        {"t131b01", 0x21B08B5B},
        {"t131b21", 0x1386E9D9},
        {"t131b23", 0xFD8888F5},
        {"t131d01 Rice Paddy Path", 0x253DF7E9},
        {"t131d03 Fullface Rock", 0xCB3396C5},
        {"t131d04 Cicada Canyon", 0x55570366},
        {"t131d05 Mt. Middleton Summit", 0x225033F0},
        {"t131g00 Harrisville", 0x507C7926},
        {"t131i01 Grandma's House", 0x2DE564BA},
        {"t131i03 Harrisville School", 0xC3EB0596},
        {"t131i05 Mountain Market", 0x2A88A0A3},
        {"t131w011", 0x188370ED},
        {"t132b01", 0x330524B5},
        {"t132b03", 0xDD0B4599},
        {"t132d01 Alley off the Plaza", 0x37885807},
        {"t132g00 Harrisville Station Plaza", 0x42C9D6C8},
        {"t201g00 Old Springdale", 0x91487158},
        {"t202g00 Old Mt. Wildwood", 0x83FDDEB6},
        {"t206g00 Gourd Pond", 0x0C9F49E1},
        {"t231g00 Old Harrisville", 0xD6E80B88},
        {"t232g00 Old Harrisville Station Plaza", 0xC45DA466},
        {"t400b01", 0x8E61075D},
        {"t400b02", 0x176856E7},
        {"t400d01 River Tootin Tributary", 0x8AEC7BEF},
        {"t400d11 Remote Relic", 0x93F74AAE},
        {"t400d13 Grumbler's Grotto - Entrance", 0x7DF92B82},
        {"t400d15 Solitary Sanctuary", 0x949A8EB7},
        {"t400d17 Forest Islet", 0x7A94EF9B},
        {"t400d19 Secret Cavern", 0x9D2CC29C},
        {"t400w011", 0xF14F0454},
        {"t400w021", 0xDA625797},
        {"t400w031", 0xC37966D6},
        {"t400w041", 0x8C38F011},
        {"t400w042", 0x1531A1AB},
        {"t400w051", 0x9523C150},
        {"t400w061", 0xBE0E9293},
        {"t400w071", 0xA715A3D2},
        {"t400w081", 0x208DBF1D},
        {"t400w082", 0xB984EEA7},
        {"t400w091", 0x39968E5C},
        {"t400w101", 0xE9965F22},
        {"t400w111", 0xF08D6E63},
        {"t401b01", 0x36DD6038},
        {"t401b03", 0xD8D30114},
        {"t401b05", 0x31B0A421},
        {"t401b07", 0xDFBEC50D},
        {"t401b11", 0x2FC65179},
        {"t401b21", 0x04EB02BA},
        {"t401b23", 0xEAE56396},
        {"t401d01 Garden Grill", 0x32501C8A},
        {"t401d03 Denton's Repairs", 0xDC5E7DA6},
        {"t401d21 Scrapyard", 0x00667E08},
        {"t401d99", 0xED492FF1},
        {"t401g00 Southmond", 0x47119245},
        {"t401i01 BBQ - Adams House - 1F", 0x3A888FD9},
        {"t401i02 BBQ - Adams House - 2F", 0xA381DE63},
        {"t401i03 Suspicious Room", 0xD486EEF5},
        {"t401i07 BBQ - Forester House - 1F", 0xD3EB2AEC},
        {"t401i08 BBQ - Forester House - 2F", 0x4354377D},
        {"t401i11 Margarita's", 0x2393BE98},
        {"t401i13 House of Boxes", 0xCD9DDFB4},
        {"t401i21 Acornia Bank", 0x08BEED5B},
        {"t401i23 Skycutter Post Office", 0xE6B08C77},
        {"t401i51 Southmond School", 0x47FF7B9C},
        {"t401i52 Southmond School - West Building", 0xDEF62A26},
        {"t401i53 Southmond School - East Building", 0xA9F11AB0},
        {"t401i55 Southmond School - Cafeteria", 0x4092BF85},
        {"t401w011", 0xCC2F2DE4},
        {"t402b01", 0x2468CFD6},
        {"t402b03", 0xCA66AEFA},
        {"t402b05", 0x23050BCF},
        {"t402b07", 0xCD0B6AE3},
        {"t402b09", 0x2AB347E4},
        {"t402b21", 0x165EAD54},
        {"t402d01 Hip & Hopping Alley", 0x20E5B364},
        {"t402d03 Warehouse No. 2", 0xCEEBD248},
        {"t402d21 Pearly Chamber of Whimsy", 0x12D3D1E6},
        {"t402d26 Whimsical Shopping - 1st Room", 0x8CB74445},
        {"t402d27 Whimsical Shopping - 2nd Room", 0xFBB074D3},
        {"t402d28 Whimsical Shopping - 3rd Room", 0x6B0F6942},
        {"t402d31 Whimsical Work - 1st Room", 0x0BC8E0A7},
        {"t402d32 Whimsical Work - 2nd Room", 0x92C1B11D},
        {"t402d33 Whimsical Work - 3rd Room", 0xE5C6818B},
        {"t402d36 Whimsical Quiz - 1st Room", 0x95AC7504},
        {"t402d37 Whimsical Quiz - 2nd Room", 0xE2AB4592},
        {"t402d38 Whimsical Quiz - 3rd Room", 0x72145803},
        {"t402d39 Whimsical Quiz - Punishment Room", 0x05136895},
        {"t402d41 Whimsical Drilling - 1st Room", 0x44897660},
        {"t402d42 Whimsical Drilling - 2nd Room", 0xDD8027DA},
        {"t402d43 Whimsical Drilling - 3rd Room", 0xAA87174C},
        {"t402d44 Whimsical Drilling - 4th Room", 0x34E382EF},
        {"t402g00 Northbeech", 0x55A43DAB},
        {"t402i01 Everymart Northbeech", 0x283D2037},
        {"t402i03 Springdale Trading - St. Peanutsburg", 0xC633411B},
        {"t402i05 Bon Voyage Boats", 0x2F50E42E},
        {"t402i07 Miracle Circus", 0xC15E8502},
        {"t402i09 Star Burger", 0x26E6A805},
        {"t402i11 Tinker's Toy Town", 0x31261176},
        {"t402i13 Sunrise Sushi", 0xDF28705A},
        {"t402i15 Amore Pizza", 0x364BD56F},
        {"t402i21 Warehouse No. 3 Marketplace", 0x1A0B42B5},
        {"t402i31 City Hall - 1F", 0x031073F4},
        {"t402i32 City Hall - 2F", 0x9A19224E},
        {"t402i33 City Hall - Rooftop", 0xED1E12D8},
        {"t403b01", 0x9CD4A8B3},
        {"t403b03", 0x72DAC99F},
        {"t403b21", 0xAEE2CA31},
        {"t403b23", 0x40ECAB1D},
        {"t403b25", 0xA98F0E28},
        {"t403d01 Hotel - Rear Byway", 0x9859D401},
        {"t403d11 Phantomart", 0x8142E540},
        {"t403d13 Phantomart - Storeroom", 0x6F4C846C},
        {"t403g00 East Pine", 0xED185ACE},
        {"t403i01 East Pine Church", 0x90814752},
        {"t403i03 Bob's Watches", 0x7E8F267E},
        {"t403i05 Steak and Pine 29", 0x97EC834B},
        {"t403i07 Old House", 0x79E2E267},
        {"t403i13 Tempura Temptations", 0x6794173F},
        {"t403i15 Succulent Sukiyaki", 0x8EF7B20A},
        {"t403i60 Wayfarer Manor - Room 101", 0xB1DCD042},
        {"t403i61 Wayfarer Manor - Room 102", 0xC6DBE0D4},
        {"t403i62 Wayfarer Manor - Room 103", 0x5FD2B16E},
        {"t403i63 Wayfarer Manor - Room 104", 0x28D581F8},
        {"t403i64 Wayfarer Manor - Room 105", 0xB6B1145B},
        {"t403i65 Wayfarer Manor - Room 201", 0xC1B624CD},
        {"t403i66 Wayfarer Manor - Room 202", 0x58BF7577},
        {"t403i67 Wayfarer Manor - Room 203", 0x2FB845E1},
        {"t403i68 Wayfarer Manor - Room 204", 0xBF075870},
        {"t403i69 Wayfarer Manor - Room 205", 0xC80068E6},
        {"t404b01", 0x0103900A},
        {"t404b03", 0xEF0DF126},
        {"t404b05", 0x066E5413},
        {"t404b07", 0xE860353F},
        {"t404d01 Hazeltine Mansion - Undergr. Passage", 0x058EECB8},
        {"t404d03 Hazeltine Mansion - Entrance Hall", 0xEB808D94},
        {"t404d05 Hazeltine Mansion - 1F Hallway", 0x02E328A1},
        {"t404d07 Hazeltine Mansion - Dining Room", 0xECED498D},
        {"t404d09 Hazeltine Mansion - Judgement Hallway", 0x0B55648A},
        {"t404d11 Hazeltine Mansion - Kitchen", 0x1C95DDF9},
        {"t404d13 Hazeltine Mansion - Bathroom", 0xF29BBCD5},
        {"t404d15 Hazeltine Mansion - 2F Hallway", 0x1BF819E0},
        {"t404d17 Hazeltine Mansion - Archive", 0xF5F678CC},
        {"t404d19 Hazeltine Mansion", 0x124E55CB},
        {"t404g00 Hazeltine Mansion", 0x70CF6277},
        {"t405b21", 0x8B8995ED},
        {"t405b23", 0x6587F4C1},
        {"t405b25", 0x8CE451F4},
        {"t405d11 Gloombell Forest", 0xA429BA9C},
        {"t405d13 Mushroom Park", 0x4A27DBB0},
        {"t405d21 Grumbler's Grotto", 0x8F04E95F},
        {"t405d23 Grumbler's Grotto", 0x610A8873},
        {"t405g00 Secret Forest Base", 0xC8730512},
        {"t405i01 Secret Base", 0xB5EA188E},
        {"t406b01", 0xAB0A5881},
        {"t406d21 Meadowbrooke Farm", 0x9DB146B1},
        {"t406g00 Meadowbrooke Farm", 0xDAC6AAFC},
        {"t411b01", 0x0BBD4988},
        {"t411b21", 0x398B2B0A},
        {"t411b31", 0x20901A4B},
        {"t411b32", 0xB9994BF1},
        {"t411b33", 0xCE9E7B67},
        {"t411b34", 0x50FAEEC4},
        {"t411b35", 0x27FDDE52},
        {"t411b36", 0xBEF48FE8},
        {"t411b37", 0xC9F3BF7E},
        {"t411b38", 0x594CA2EF},
        {"t411b39", 0x2E4B9279},
        {"t411b40", 0x18D6BC1A},
        {"t411d00 Hopper's Gorge", 0x783705AC},
        {"t411d10 Hopper's Gorge - Condor Canyon", 0x612C34ED},
        {"t411d21 Tower of Zenlightenment - 1st Trial", 0x3D0657B8},
        {"t411d22 Tower of Zenlightenment - 2nd Trial", 0xA40F0602},
        {"t411d23 Tower of Zenlightenment - 3rd Trial", 0xD3083694},
        {"t411d24 Tower of Zenlightenment - 4th Trial", 0x4D6CA337},
        {"t411d25 Tower of Zenlightenment - 5th Trial", 0x3A6B93A1},
        {"t411d26 Tower of Zenlightenment - 6th Trial", 0xA362C21B},
        {"t411d27 Tower of Zenlightenment - 7th Trial", 0xD465F28D},
        {"t411d28 Tower of Zenlightenment - 8th Trial", 0x44DAEF1C},
        {"t411d29 Tower of Zenlightenment - 9th Trial", 0x33DDDF8A},
        {"t411d30 Tower of Zenlightenment - Final Trial", 0x531A566F},
        {"t411d41 Ascension Ground", 0x6B5CF03E},
        {"t411g00 Dukesville", 0x7A71BBF5},
        {"t411i01 Outlaw Hotel", 0x07E8A669},
        {"t411i03 Sheriff's Office", 0xE9E6C745},
        {"t411i05 Wild Hunters", 0x00856270},
        {"t412b01", 0x1908E666},
        {"t412g00 Pecan Port", 0x68C4141B},
        {"t490d01 Suspicious Room", 0x87FC199E},
        {"t491d01", 0x3F407EFB},
        {"t491o01", 0x3315911A},
        {"t501d01 Banquet Room", 0xF90CCF2F},
        {"t501g00 Cluvian Continent", 0x8C4D41E0},
        {"t501i01 Cluvian Continent - Blasters Camp", 0xF1D45C7C},
        {"t581d01 Cluvian Keyhole - Entrance", 0xC97C84EE},
        {"t581d02 Tomb of King Clupharaon - Entr.", 0x5075D554},
        {"t581d03 Temple of Clusis - Entrance", 0x2772E5C2},
        {"t581d04 Statue of Clunubis - Entrance", 0xB9167061},
        {"t581d05 Pyramid of Clu - Entrance", 0xCE1140F7},
        {"t581d06 Mt. Cluvimpus - Entrance", 0x5718114D},
        {"t581d07 Cluphinx - Entrance", 0x201F21DB},
        {"t581d08 Tower of Clu - Entrance", 0xB0A03C4A},
        {"t581d09 Enma Palace - Entrance", 0xC7A70CDC},
        {"t581d12 Sky Garden - Entrance", 0x496EE415},
        {"t591d01", 0xF41CAD5E},
        {"t591o01", 0xF84942BF},
        {"t591o02", 0x61401305},
        {"t592d01", 0xE6A902B0},
        {"t592d02", 0x7FA0530A},
        {"t592d03", 0x08A7639C},
        {"t592d04", 0x96C3F63F},
        {"t592d05", 0xE1C4C6A9},
        {"t592d06", 0x78CD9713},
        {"t592d07", 0x0FCAA785},
        {"t592d08", 0x9F75BA14},
        {"t592d10", 0x88B50367},
        {"t592d11", 0xFFB233F1},
        {"t592d12", 0x66BB624B},
        {"t592o01", 0xEAFCED51},
        {"t592o02", 0x73F5BCEB},
        {"t592o03", 0x04F28C7D},
        {"t592o04", 0x9A9619DE},
        {"t592o05", 0xED912948},
        {"t592o06", 0x749878F2},
        {"t592o07", 0x039F4864},
        {"t592o08", 0x932055F5},
        {"t592o09", 0xE4276563},
        {"t592o10 Enma Palace", 0x84E0EC86},
        {"t592o11", 0xF3E7DC10},
        {"t592o12", 0x6AEE8DAA},
        {"testmap", 0xEBB16BD2},
        {"testmappl", 0x438CB286},
        {"testmappl02", 0x1EFEDB6D},
        {"testmappl03", 0x69F9EBFB},
        {"testmappl04", 0xF79D7E58},
        {"testmappl05", 0x809A4ECE},
        {"testmappl06", 0x19931F74},
        {"testmappl07", 0x6E942FE2},
        {"testmappl08", 0xFE2B3273},
        {"testmappl09", 0x892C02E5},
        {"testmappl10", 0xE9EB8B00},
    };
    static const u32 g_mapCount = sizeof(g_maps) / sizeof(g_maps[0]);

    // ---- 状態(New3DS専用) ----
    static u32         g_destId   = 0x91487158;
    static std::string g_destName = "t201g00 Old Springdale [unused]";
    static int         g_char     = 0;           // 0 = ケータ, 1 = イナホ, 2 = 手入力アドレス
    static u32         g_custom   = 0x0C3266E4;
    static int         g_hotkey   = 5;           // 5 = A+B (ARコードと同じ)
    static bool        g_trigOn   = true;        // ARコードの1ブロック目(floatに加算)を実行するか
    static u32         g_trigAddr = 0x0C4F09C0;

    // New3DS用ARコードにあった、現在のマップIDの枠
    static const u32 NEW_KEITA = 0x0C3266E4;
    static const u32 NEW_INAHO = 0x0C3268F4;

    static u32 HotkeyMask(int idx)
    {
        switch (idx)
        {
            case 0:  return Key::L | Key::R;
            case 1:  return Key::L | Key::DPadUp;
            case 2:  return Key::L | Key::DPadDown;
            case 3:  return Key::L | Key::DPadLeft;
            case 4:  return Key::L | Key::DPadRight;
            case 5:  return Key::A | Key::B;
            default: return Key::X | Key::Y;
        }
    }

    static const char *MapNameOf(u32 id)
    {
        for (u32 i = 0; i < g_mapCount; i++)
            if (g_maps[i].id == id)
                return g_maps[i].name;
        return nullptr;
    }

    static std::string Hex8(u32 v)
    {
        char b[16];
        snprintf(b, sizeof(b), "%08X", (unsigned)v);
        return std::string(b);
    }

    static std::string Dec(u32 v)
    {
        char b[16];
        snprintf(b, sizeof(b), "%u", (unsigned)v);
        return std::string(b);
    }

    // アドレスの今の値と、一覧にあるマップ名を文字列にする(診断用)
    static std::string DescribeAddr(u32 addr)
    {
        u32 v = 0;
        if (!Process::Read32(addr, v))
            return Hex8(addr) + ": READ FAILED";
        const char *n = MapNameOf(v);
        return Hex8(addr) + ": " + Hex8(v) + (n ? std::string(" = ") + n : std::string(" (not in list)"));
    }

    static void ShowInfo(const std::string &text)
    {
        std::vector<std::string> ok = {"OK"};
        Keyboard k(text, ok);
        k.Open();
    }

    static void ShowCheck(void)
    {
        ShowInfo("Current values (stand in a normal map first):\n"
                 "Keita " + DescribeAddr(NEW_KEITA) + "\n"
                 "Inaho " + DescribeAddr(NEW_INAHO) + "\n"
                 "Custom " + DescribeAddr(g_custom) + "\n"
                 "Trigger " + DescribeAddr(g_trigAddr) + " (float)");
    }

    static u32 TargetAddr(void)
    {
        return (g_char == 0) ? NEW_KEITA : (g_char == 1 ? NEW_INAHO : g_custom);
    }

    // ARコードの1ブロック目: *(g_trigAddr) += (float)0xDD000000
    static void DoTrigger(void)
    {
        u32 raw = 0;
        if (!Process::Read32(g_trigAddr, raw))
            return;
        float f;
        memcpy(&f, &raw, sizeof(f));
        u32 addBits = 0xDD000000;
        float a;
        memcpy(&a, &addBits, sizeof(a));
        f += a;
        memcpy(&raw, &f, sizeof(raw));
        Process::Write32(g_trigAddr, raw);
    }

    static bool g_wasDown   = false;
    static u32  g_lastShown = 0;

    // 毎フレーム呼ばれる。ホットキーを押している間だけ実行する(ARの DD000000 と同じ)
    // 診断: 書き込む直前の値を読み、押した瞬間と「ゲームが値を書き換えた時」だけ画面に出す
    static void MapWarpFrame(MenuEntry *entry)
    {
        bool down = Controller::IsKeysDown(HotkeyMask(g_hotkey));
        if (down)
        {
            u32 addr = TargetAddr();

            u32 pre = 0;
            bool rd = Process::Read32(addr, pre);
            if (rd && (!g_wasDown || (pre != g_destId && pre != g_lastShown)))
            {
                const char *n = MapNameOf(pre);
                OSD::Notify(std::string(g_wasDown ? "game changed slot: " : "slot before: ") + Hex8(pre) + (n ? std::string(" ") + n : std::string("")));
                g_lastShown = pre;
            }

            if (g_trigOn)
                DoTrigger();
            bool ok = Process::Write32(addr, g_destId);
            if (!g_wasDown)
                OSD::Notify(std::string(ok ? "write ok  " : "write FAILED  ") + DescribeAddr(addr));
        }
        g_wasDown = down;
    }

    // 0=g(フィールド) 1=i(屋内) 2=d(ダンジョン) 3=b(戦闘) 4=s(駅) 5=e(イベント) 6=その他
    static int MapCategory(const char *n)
    {
        if (n[0] == 'e')
            return 5;
        if (n[0] == 't' && strlen(n) > 4)
        {
            switch (n[4])
            {
                case 'g': return 0;
                case 'i': return 1;
                case 'd': return 2;
                case 'b': return 3;
                case 's': return 4;
            }
        }
        return 6;
    }

    static void SelectDestination(void)
    {
        std::vector<std::string> cats =
        {
            "Field (g)", "Interior (i)", "Dungeon etc. (d)", "Battle (b)",
            "Station (s)", "Event (e)", "Other (w/o/t/test)", "Hex input"
        };
        Keyboard kb("Category", cats);
        int c = kb.Open();
        if (c < 0)
            return;

        if (c == 7)
        {
            u32 v = g_destId;
            Keyboard hex("Map ID (hex)");
            hex.IsHexadecimal(true);
            if (hex.Open(v, g_destId) >= 0)
            {
                g_destId = v;
                g_destName = "(hex input)";
                OSD::Notify("Map ID set");
            }
            return;
        }

        std::vector<u32>         idx;
        std::vector<std::string> names;
        for (u32 i = 0; i < g_mapCount; i++)
        {
            if (MapCategory(g_maps[i].name) == c)
            {
                idx.push_back(i);
                names.push_back(g_maps[i].name);
            }
        }

        Keyboard list("Select map", names);
        int s = list.Open();
        if (s < 0 || s >= (int)idx.size())
            return;

        g_destId = g_maps[idx[s]].id;
        g_destName = g_maps[idx[s]].name;
        OSD::Notify("Dest: " + g_destName);
    }

    // メニュー(エントリの設定ボタン)から呼ばれる
    static void MapWarpMenu(MenuEntry *entry)
    {
        static const char *hotkeyNames[] =
        {
            "L+R", "L+Up", "L+Down", "L+Left", "L+Right", "A+B", "X+Y"
        };
        static const char *charNames[] = {"Keita", "Inaho", "Custom address"};

        for (;;)
        {
            std::vector<std::string> opts =
            {
                "Destination: " + g_destName,
                std::string("Character: ") + charNames[g_char] + " (" + Hex8(TargetAddr()) + ")",
                std::string("Hotkey: ") + hotkeyNames[g_hotkey],
                "Check: show current values",
                std::string("Trigger (float add, AR block 1): ") + (g_trigOn ? "ON" : "OFF"),
                "Custom address (hex): " + Hex8(g_custom),
                "Trigger address (hex): " + Hex8(g_trigAddr),
            };
            Keyboard kb("Map Warp (hold hotkey, then wait for the load)", opts);
            int r = kb.Open();
            if (r < 0)
                return;

            if (r == 0)
                SelectDestination();
            else if (r == 1)
            {
                std::vector<std::string> cs(charNames, charNames + 3);
                Keyboard ck("Character", cs);
                int cr = ck.Open();
                if (cr >= 0) g_char = cr;
            }
            else if (r == 2)
            {
                std::vector<std::string> hk(hotkeyNames, hotkeyNames + 7);
                Keyboard hk2("Hotkey", hk);
                int hr = hk2.Open();
                if (hr >= 0) g_hotkey = hr;
            }
            else if (r == 3)
            {
                ShowCheck();
            }
            else if (r == 4)
            {
                g_trigOn = !g_trigOn;
            }
            else if (r == 5)
            {
                u32 v = g_custom;
                Keyboard hex("Custom address (hex)");
                hex.IsHexadecimal(true);
                if (hex.Open(v, g_custom) >= 0)
                    g_custom = v;
            }
            else if (r == 6)
            {
                u32 v = g_trigAddr;
                Keyboard hex("Trigger address (hex)");
                hex.IsHexadecimal(true);
                if (hex.Open(v, g_trigAddr) >= 0)
                    g_trigAddr = v;
            }
        }
    }

    // ================= 移動系(最初の3gxの「移動系」から) =================
    // 空中歩行 : 0x006D23E0 のコードを 0xEA000021 (分岐命令)に書き換える(切る時は元の値に戻す)
    // 落下速度低下 : 0x0C4D57D0 のバイトに 1 を毎フレーム書く
    // ムーンジャンプ : プレイヤーの高さ(float, 0x0C4F09C0)を ZR で足し、ZL で引く
    static const u32 AIR_ADDR      = 0x006D23E0;
    static const u32 AIR_PATCH     = 0xEA000021;
    static const u32 SLOWFALL_ADDR = 0x0C4D57D0;
    static const u32 PLAYER_X_ADDR = 0x0C4F09BC; // 十字キー左右
    static const u32 PLAYER_Y_ADDR = 0x0C4F09C0; // 高さ
    static const u32 PLAYER_Z_ADDR = 0x0C4F09C4; // 十字キー上下

    static bool g_airWalk   = false;
    static bool g_airSaved  = false;
    static u32  g_airOrig   = 0;
    static bool g_slowFall  = false;
    static bool g_moonJump  = false;
    static u32  g_moonSpeed = 5; // 元のプラグインの初期値
    static bool g_coordMove = false;
    static u32  g_coordSpeed = 5; // 元のプラグインの初期値

    static void SetAirWalk(bool on)
    {
        if (on)
        {
            u32 cur = 0;
            if (Process::Read32(AIR_ADDR, cur) && cur != AIR_PATCH)
            {
                g_airOrig = cur;
                g_airSaved = true;
            }
            g_airWalk = true;
        }
        else
        {
            g_airWalk = false;
            if (g_airSaved)
                Process::Write32(AIR_ADDR, g_airOrig);
        }
    }

    static void AddToFloat(u32 addr, float delta)
    {
        u32 raw = 0;
        if (!Process::Read32(addr, raw))
            return;
        float f;
        memcpy(&f, &raw, sizeof(f));
        f += delta;
        memcpy(&raw, &f, sizeof(raw));
        Process::Write32(addr, raw);
    }

    // Controller::IsKeyDown は数値ではなく Key 型を受け取るので、数値のキーマスクを変換して呼ぶ
    static bool KeyDown(u32 mask)
    {
        return Controller::IsKeyDown((Key)mask);
    }

    // 毎フレーム呼ばれる(エントリを有効にしている間)
    static void MoveFrame(MenuEntry *entry)
    {
        if (g_airWalk)
            Process::Write32(AIR_ADDR, AIR_PATCH);
        if (g_slowFall)
            Process::Write8(SLOWFALL_ADDR, 1);
        if (g_coordMove && KeyDown(0x02)) // B + 十字キーで座標を直接動かす(壁抜け)
        {
            float cs = (float)g_coordSpeed;
            if (KeyDown(0x10)) AddToFloat(PLAYER_X_ADDR,  cs); // 右
            if (KeyDown(0x20)) AddToFloat(PLAYER_X_ADDR, -cs); // 左
            if (KeyDown(0x80)) AddToFloat(PLAYER_Z_ADDR,  cs); // 下
            if (KeyDown(0x40)) AddToFloat(PLAYER_Z_ADDR, -cs); // 上
        }
        if (g_moonJump)
        {
            float sp = (float)g_moonSpeed;
            if (KeyDown(0x8000))      // ZR: 上昇
                AddToFloat(PLAYER_Y_ADDR, sp);
            else if (KeyDown(0x4000)) // ZL: 下降(元は速度の2倍)
                AddToFloat(PLAYER_Y_ADDR, -2.0f * sp);
        }
    }

    static void MoveMenu(MenuEntry *entry)
    {
        for (;;)
        {
            std::vector<std::string> opts =
            {
                std::string("Air walk: ") + (g_airWalk ? "ON" : "OFF"),
                std::string("Slow fall: ") + (g_slowFall ? "ON" : "OFF"),
                std::string("Moon jump (ZR up / ZL down): ") + (g_moonJump ? "ON" : "OFF"),
                "Moon jump speed: " + Dec(g_moonSpeed),
                std::string("Coordinate move / noclip (B + D-pad): ") + (g_coordMove ? "ON" : "OFF"),
                "Coordinate move speed: " + Dec(g_coordSpeed),
            };
            Keyboard kb("Movement (entry must be enabled)", opts);
            int r = kb.Open();
            if (r < 0)
                return;

            if (r == 0)
                SetAirWalk(!g_airWalk);
            else if (r == 1)
                g_slowFall = !g_slowFall;
            else if (r == 2)
                g_moonJump = !g_moonJump;
            else if (r == 3)
            {
                u32 v = g_moonSpeed;
                Keyboard sk("Moon jump speed (default 5)");
                if (sk.Open(v, g_moonSpeed) >= 0 && v > 0 && v < 1000)
                    g_moonSpeed = v;
            }
            else if (r == 4)
                g_coordMove = !g_coordMove;
            else if (r == 5)
            {
                u32 v = g_coordSpeed;
                Keyboard sk("Coordinate move speed (default 5)");
                if (sk.Open(v, g_coordSpeed) >= 0 && v > 0 && v < 1000)
                    g_coordSpeed = v;
            }
        }
    }

    void PatchProcess(FwkSettings &settings)
    {
    }

    void OnProcessExit(void)
    {
    }

    int main(void)
    {
        PluginMenu *menu = new PluginMenu("YW3 Map Warp", 0, 1, 0,
            "Yo-kai Watch 3 Sukiyaki map warp.\n"
            "Enable the entry, open its settings, pick a destination, "
            "then hold the hotkey and go through a door / loading trigger.");

        menu->SynchronizeWithFrame(true);

        menu->Append(new MenuEntry("Map Warp", MapWarpFrame, MapWarpMenu,
            "Writes the chosen map ID to the next-map address while the hotkey is held."));

        menu->Append(new MenuEntry("Movement", MoveFrame, MoveMenu,
            "Air walk / slow fall / moon jump (ZR up, ZL down) / B + D-pad coordinate move. Enable, then open settings."));

        menu->Run();
        delete menu;
        return 0;
    }
}
