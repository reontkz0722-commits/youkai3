// 妖怪ウォッチ3 スキヤキ マップワープ (CTRPluginFramework)
// ※未ビルド・未テスト。Nanquitas の CTRPluginFrameworkBlankTemplate の Sources/main.cpp と差し替えて使う。
//
// 動作: メニューで行き先とキャラを選び、ホットキーを押している間、
//       次マップIDのアドレスへ行き先IDを書き込む(ARコードと同じ動き)。
// アドレス(New3DS用・wikiのNEW用ARコード): オフセット指定なしの絶対アドレス
//   ケータ : 0x0C3266E4
//   イナホ : 0x0C3268F4
// Old3DS用は D3000000 08000000 + 007266E4(ケータ) / 007268F4(イナホ)
// ※ ARコードの先頭ブロック(F0000001 / F1000000 …)は、書き込みのない空ブロックなので省いている

#include <3ds.h>
#include "csvc.h"
#include <CTRPluginFramework.hpp>
#include <cstdio>
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
        // ---- フィールド ----
        {"t001g00 New Yo-kai City", 0xDC80D053},
        {"t101g00 Uptown Springdale", 0x17DC03F6},
        {"t102g00 Mt. Wildwood", 0x0569AC18},
        {"t103g00 Blossom Heights", 0xBDD5CB7D},
        {"t104g00 Downtown Springdale", 0x2002F3C4},
        {"t105g00 Shopper's Row", 0x98BE94A1},
        {"t106g00 Breezy Hills", 0x8A0B3B4F},
        {"t107g00 Excellent Tower", 0x32B75C2A},
        {"t108g00 Sparkopolis", 0x6AD44C7C},
        {"t109g00 Greenfields Station", 0xD2682B19},
        {"t121g00 San Fantastico", 0x6D1C5096},
        {"t131g00 Harrisville", 0x507C7926},
        {"t132g00 Harrisville Stn Plaza", 0x42C9D6C8},
        {"t201g00 Old Springdale [unused]", 0x91487158},
        {"t202g00 Old Mt. Wildwood [unused]", 0x83FDDEB6},
        {"t206g00 Gourd Pond [unused]", 0x0C9F49E1},
        {"t231g00 Old Harrisville [unused]", 0xD6E80B88},
        {"t232g00 Old Harrisville Plaza [unused]", 0xC45DA466},
        {"t401g00 Southmond", 0x47119245},
        {"t402g00 Northbeech", 0x55A43DAB},
        {"t403g00 East Pine", 0xED185ACE},
        {"t404g00 Hazeltine Mansion", 0x70CF6277},
        {"t405g00 Secret Forest Base", 0xC8730512},
        {"t406g00 Meadowbrooke Farm", 0xDAC6AAFC},
        {"t411g00 Dukesville", 0x7A71BBF5},
        {"t412g00 Pecan Port", 0x68C4141B},
        {"t501g00 Cluvian Continent", 0x8C4D41E0},

        // ---- 屋内 ----
        {"t100i21 Mirror of Truth", 0xE0CF1B8D},
        {"t100i23 ?", 0x0EC17AA1},
        {"t100i25 ?", 0xE7A2DF94},
        {"t101i01 Adams House 1F", 0x6A451E6A},
        {"t101i02 Adams House 2F", 0xF34C4FD0},
        {"t101i03 Forester House 1F", 0x844B7F46},
        {"t101i04 Forester House 2F", 0x1A2FEAE5},
        {"t101i10 Logan's House", 0x04591FBD},
        {"t101i21 Banter Bakery", 0x58737CE8},
        {"t101i23 Everymart Uptown", 0xB67D1DC4},
        {"t101i25 Community Center", 0x5F1EB8F1},
        {"t101i27 Piggleston Bank", 0xB110D9DD},
        {"t101i29 Lambert Post Office", 0x56A8F4DA},
        {"t101i31 Jungle Hunter", 0x41684DA9},
        {"t101i51 Elementary 1F South", 0x1732EA2F},
        {"t101i52 Elementary 1F North", 0x8E3BBB95},
        {"t101i53 Elementary 2F", 0xF93C8B03},
        {"t101i55 Elementary 3F", 0x105F2E36},
        {"t101i58 Elementary Roof", 0x6EEE528B},
        {"t101i59 Elementary Gym", 0x19E9621D},
        {"t102i01 Deserted House", 0x78F0B184},
        {"t102i21 Shrine Behind Waterfall", 0x4AC6D306},
        {"t103i01 Bernstein House 1F", 0xC04CD6E1},
        {"t103i02 Bernstein House 2F", 0x5945875B},
        {"t103i03 Bernstein House 3F", 0x2E42B7CD},
        {"t103i10 Megan's House", 0xAE50D736},
        {"t103i21 Timers & More", 0xF27AB463},
        {"t103i23 Candy Stop", 0x1C74D54F},
        {"t103i25 Everymart Blossom Hts", 0xF517707A},
        {"t103i31 Shoten Temple", 0xEB618522},
        {"t103i33 Prayer's Peak Tunnel", 0x056FE40E},
        {"t103i35 Chloro-Phil Good", 0xEC0C413B},
        {"t103i37 Hot Springs Lobby", 0x02022017},
        {"t103i38 Hot Springs Men", 0x92BD3D86},
        {"t103i39 Hot Springs Women", 0xE5BA0D10},
        {"t103i51 Byrd House Mayan Room", 0xBD3B22A4},
        {"t103i53 Byrd House Hidden Room", 0x53354388},
        {"t103i60 Wayfarer Manor 101", 0xE11141F1},
        {"t103i61 Wayfarer Manor 102", 0x96167167},
        {"t103i62 Wayfarer Manor 103", 0x0F1F20DD},
        {"t103i63 Wayfarer Manor 104", 0x7818104B},
        {"t103i64 Wayfarer Manor 105", 0xE67C85E8},
        {"t103i65 Wayfarer Manor 201", 0x917BB57E},
        {"t103i66 Wayfarer Manor 202", 0x0872E4C4},
        {"t103i67 Wayfarer Manor 203", 0x7F75D452},
        {"t103i68 Wayfarer Manor 204", 0xEFCAC9C3},
        {"t103i69 Wayfarer Manor VIP", 0x98CDF955},
        {"t104i10 Springdale Underground", 0x3387EF8F},
        {"t104i21 Seabreeze Tunnel", 0x6FAD8CDA},
        {"t104i23 Frostia's Place", 0x81A3EDF6},
        {"t104i25 Arcadia Arcade", 0x68C048C3},
        {"t104i27 Nom Burger", 0x86CE29EF},
        {"t104i29 Fortune Hospital 1F", 0x617604E8},
        {"t104i30 Fortune Hospital 2F", 0x01B18D0D},
        {"t104i31 Foundation Academy", 0x76B6BD9B},
        {"t104i33 Everymart Downtown", 0x98B8DCB7},
        {"t104i35 Cafe Shanista", 0x71DB7982},
        {"t104i37 Sports Club 1F", 0x9FD518AE},
        {"t104i38 Sports Club 2F", 0x0F6A053F},
        {"t104i39 Sports Club 3F", 0x786D35A9},
        {"t104i41 Belly Buster Curry", 0x39F72B5C},
        {"t104i43 Tortoise Camera Shop", 0xD7F94A70},
        {"t104i45 Sunset Mall B1", 0x3E9AEF45},
        {"t104i47 Sunset Mall 1F", 0xD0948E69},
        {"t104i49 Sunset Mall Dolphin Cafe", 0x372CA36E},
        {"t104i51 Business Tower 1F", 0x20EC1A1D},
        {"t104i53 Business Tower 7F", 0xCEE27B31},
        {"t104i55 Sumptuous Sukiyaki", 0x2781DE04},
        {"t104i61 Springdale Central Stn", 0x0BC149DE},
        {"t105i20 Flower Road", 0xA016DB29},
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
        {"t106i01 Archer House 1F", 0xF79226D3},
        {"t106i02 Archer House 2F", 0x6E9B7769},
        {"t106i05 Stone House", 0xF0FFE2CA},
        {"t106i07 Amy's House 1F", 0x1EF183E6},
        {"t106i09 Amy's House 2F", 0xF949AEE1},
        {"t106i10 Lina's House", 0x998E2704},
        {"t106i11 Thomas House", 0xEE891792},
        {"t106i13 Breezy Hills Apts Ent.", 0x008776BE},
        {"t106i15 Breezy Hills Apts 7F", 0xE9E4D38B},
        {"t106i21 Everymart Breezy Hills", 0xC5A44451},
        {"t106i23 Trophy Room", 0x2BAA257D},
        {"t106i51 Wisteria Gardens Parking", 0x8AE5D296},
        {"t106i52 Wisteria Gardens Ent.", 0x13EC832C},
        {"t106i55 Stonewood House", 0x8D88168F},
        {"t107i01 Excellent Tower Ent.", 0x4F2E41B6},
        {"t107i02 Excellent Tower Elevator", 0xD627100C},
        {"t107i03 Excellent Tower Obs. Deck", 0xA120209A},
        {"t108i01 Next HarMEOWny Ent.", 0x174D51E0},
        {"t108i02 Next HarMEOWny Stage", 0x8E44005A},
        {"t108i05 AnimeChum", 0x102095F9},
        {"t108i07 Maid in Heaven", 0xFE2EF4D5},
        {"t108i09 Everymart Sparkopolis", 0x1996D9D2},
        {"t108i11 Detective Agency", 0x0E5660A1},
        {"t108i13 Soba Sensation", 0xE058018D},
        {"t121i01 Rolling Waves Hall", 0x10854D0A},
        {"t121i03 Rusty's Mart", 0xFE8B2C26},
        {"t121i10 Deserted House", 0x7E994CDD},
        {"t121i20 Sea", 0x55B41F1E},
        {"t131i01 Grandma's House", 0x2DE564BA},
        {"t131i03 Harrisville School", 0xC3EB0596},
        {"t131i05 Mountain Market", 0x2A88A0A3},
        {"t401i01 BBQ Adams House 1F", 0x3A888FD9},
        {"t401i02 BBQ Adams House 2F", 0xA381DE63},
        {"t401i03 Suspicious Room", 0xD486EEF5},
        {"t401i07 BBQ Forester House 1F", 0xD3EB2AEC},
        {"t401i08 BBQ Forester House 2F", 0x4354377D},
        {"t401i11 Margarita's", 0x2393BE98},
        {"t401i13 House of Boxes", 0xCD9DDFB4},
        {"t401i21 Acornia Bank", 0x08BEED5B},
        {"t401i23 Skycutter Post Office", 0xE6B08C77},
        {"t401i51 Southmond School", 0x47FF7B9C},
        {"t401i52 Southmond School West", 0xDEF62A26},
        {"t401i53 Southmond School East", 0xA9F11AB0},
        {"t401i55 Southmond Cafeteria", 0x4092BF85},
        {"t402i01 Everymart Northbeech", 0x283D2037},
        {"t402i03 Springdale Trading", 0xC633411B},
        {"t402i05 Bon Voyage Boats", 0x2F50E42E},
        {"t402i07 Miracle Circus", 0xC15E8502},
        {"t402i09 Star Burger", 0x26E6A805},
        {"t402i11 Tinker's Toy Town", 0x31261176},
        {"t402i13 Sunrise Sushi", 0xDF28705A},
        {"t402i15 Amore Pizza", 0x364BD56F},
        {"t402i21 Warehouse No. 3", 0x1A0B42B5},
        {"t402i31 City Hall 1F", 0x031073F4},
        {"t402i32 City Hall 2F", 0x9A19224E},
        {"t402i33 City Hall Rooftop", 0xED1E12D8},
        {"t403i01 East Pine Church", 0x90814752},
        {"t403i03 Bob's Watches", 0x7E8F267E},
        {"t403i05 Steak and Pine 29", 0x97EC834B},
        {"t403i07 Old House", 0x79E2E267},
        {"t403i13 Tempura Temptations", 0x6794173F},
        {"t403i15 Succulent Sukiyaki", 0x8EF7B20A},
        {"t403i60 Wayfarer Manor 101 (EP)", 0xB1DCD042},
        {"t403i61 Wayfarer Manor 102 (EP)", 0xC6DBE0D4},
        {"t403i62 Wayfarer Manor 103 (EP)", 0x5FD2B16E},
        {"t403i63 Wayfarer Manor 104 (EP)", 0x28D581F8},
        {"t403i64 Wayfarer Manor 105 (EP)", 0xB6B1145B},
        {"t403i65 Wayfarer Manor 201 (EP)", 0xC1B624CD},
        {"t403i66 Wayfarer Manor 202 (EP)", 0x58BF7577},
        {"t403i67 Wayfarer Manor 203 (EP)", 0x2FB845E1},
        {"t403i68 Wayfarer Manor 204 (EP)", 0xBF075870},
        {"t403i69 Wayfarer Manor 205 (EP)", 0xC80068E6},
        {"t405i01 Secret Base", 0xB5EA188E},
        {"t411i01 Outlaw Hotel", 0x07E8A669},
        {"t411i03 Sheriff's Office", 0xE9E6C745},
        {"t411i05 Wild Hunters", 0x00856270},
        {"t001i51 Club Koma", 0xDC6E398A},
        {"t001i53 Enma Villa", 0x326058A6},
        {"t001i55 Yopple Store", 0xDB03FD93},
        {"t001i57 Phoenix Sushi", 0x350D9CBF},
        {"t001i59 Serenity Tempura", 0xD2B5B1B8},
        {"t001i60 Blasters House B1", 0x80445ADF},
        {"t001i61 Blasters House", 0xF7436A49},
        {"t001i71 Haven Sukiyaki", 0xEE585B08},
        {"t501i01 Blasters Camp", 0xF1D45C7C},

        // ---- ダンジョン等 ----
        {"t001d41 Ambrosia Pavilion Ent.", 0xCDAD9B98},
        {"t001d42 Yo-kai World Entry Hall", 0x54A4CA22},
        {"t001d46 Ambrosia Pavilion", 0x53C90E3B},
        {"t001d49 Treasure Ship 7 Gods", 0xC37613AA},
        {"t001d21 Yopple HQ Garden", 0x9BF73C1E},
        {"t001d23 Yopple HQ Lobby", 0x75F95D32},
        {"t001d25 Yopple HQ CEO Office", 0x9C9AF807},
        {"t001d27 Yopple HQ Watch Center", 0x7294992B},
        {"t001d29 Yopple HQ Meeting Room", 0x952CB42C},
        {"t001d31 Yopple HQ Lab", 0x82EC0D5F},
        {"t001d33 Yopple HQ Assembly Line", 0x6CE26C73},
        {"t001d35 Yopple HQ Fighting Room", 0x8581C946},
        {"t001d37 Yopple HQ Cafeteria", 0x6B8FA86A},
        {"t001d39 Yopple HQ Fish Tank", 0x8C37856D},
        {"t001d51 Bada-Bing Lobby", 0xD4B6AAD9},
        {"t001d53 Bada-Bing Elevator", 0x3AB8CBF5},
        {"t001d55 Dangerous Floor", 0xD3DB6EC0},
        {"t001d57 Dream Floor Moon", 0x3DD50FEC},
        {"t001d59 Dream Floor Fire", 0xDA6D22EB},
        {"t001d61 Dream Floor Water", 0xFF9BF91A},
        {"t001d63 Dream Floor Woods", 0x11959836},
        {"t001d64 Dream Floor Woods 2", 0x8FF10D95},
        {"t001d65 Dream Floor Gold", 0xF8F63D03},
        {"t001d66 Dream Floor Gold 2", 0x61FF6CB9},
        {"t001d67 Dream Floor Earth", 0x16F85C2F},
        {"t001d69 Dream Floor Sun", 0xF1407128},
        {"t001d71 Ghoulfather's Office", 0xE680C85B},
        {"t001d73 UFO Interior", 0x088EA977},
        {"t100d00 (unnamed)", 0xAD26DACA},
        {"t100d01 Sewers Mouse Alley", 0xDA21EA5C},
        {"t100d02 Sewers Secret Space", 0x4328BBE6},
        {"t100d03 Sewers Cat Square", 0x342F8B70},
        {"t100d04 Sewers Neon Town", 0xAA4B1ED3},
        {"t100d05 Sewers Main Street", 0xDD4C2E45},
        {"t100d06 Sewers School Route", 0x44457FFF},
        {"t100d07 Sewers Kaos March Mural", 0x33424F69},
        {"t100d08 Sewers Bracing Path", 0xA3FD52F8},
        {"t100d09 Sewers Breezy Hills Byway", 0xD4FA626E},
        {"t100d10 Sewers Frogetmenot's Inn", 0xB43DEB8B},
        {"t100d11 Sparkopolis Sewers", 0xC33ADB1D},
        {"t100d12 Sewers Yo-kai Parade Mural", 0x5A338AA7},
        {"t100d13 Sewers Downtown Westside", 0x2D34BA31},
        {"t101d01 Shady Back Alley", 0x629D8D39},
        {"t101d02 Lonely Waterway", 0xFB94DC83},
        {"t101d03 The Catwalk", 0x8C93EC15},
        {"t101d05 Desolate Lane", 0x65F04920},
        {"t102d01 Mt. Wildwood Path", 0x702822D7},
        {"t102d02 Mt. Wildwood Summit", 0xE921736D},
        {"t102d03 Jumbo Slider", 0x9E2643FB},
        {"t102d31 Abandoned Tunnel West", 0x5B057114},
        {"t102d32 Abandoned Tunnel East", 0xC20C20AE},
        {"t103d01 Tucked Away Lot", 0xC89445B2},
        {"t103d03 Hidden Side Street", 0x269A249E},
        {"t103d11 Secret Byway", 0xD18F74F3},
        {"t103d31 Old Mansion Main House", 0xE3B91671},
        {"t103d33 Old Mansion Side House", 0x0DB7775D},
        {"t103d35 Old Mansion Main Attic", 0xE4D4D268},
        {"t103d36 Old Mansion Side Attic", 0x7DDD83D2},
        {"t104d01 Academy Shortcut", 0x55437D0B},
        {"t104d03 Behind Frostia's Place", 0xBB4D1C27},
        {"t104d05 Delivery Bay", 0x522EB912},
        {"t104d07 Underground Parking", 0xBC20D83E},
        {"t104d11 Business Tower 4F", 0x4C584C4A},
        {"t104d13 Business Tower 13F", 0xA2562D66},
        {"t104d15 Business Tower East Stairs", 0x4B358853},
        {"t104d17 Business Tower West Stairs", 0xA53BE97F},
        {"t104d21 Business Tower CEO Office", 0x67751F89},
        {"t105d01 Shopping Street Narrows", 0xEDFF1A6E},
        {"t105d11 Tranquility A-102", 0xF4E42B2F},
        {"t105d12 Tranquility A-104", 0x6DED7A95},
        {"t105d13 Tranquility A-201", 0x1AEA4A03},
        {"t105d14 Tranquility A-303", 0x848EDFA0},
        {"t105d15 Tranquility B-202", 0xF389EF36},
        {"t105d16 Tranquility B-204", 0x6A80BE8C},
        {"t105d17 Tranquility B-301", 0x1D878E1A},
        {"t105d18 Tranquility B-302", 0x8D38938B},
        {"t105d19 Tranquility C-102", 0xFA3FA31D},
        {"t105d20 Tranquility B-102", 0xA8CE487A},
        {"t105d21 Tranquility B-101", 0xDFC978EC},
        {"t105d22 Tranquility C-104", 0x46C02956},
        {"t105d23 Tranquility A-202", 0x31C719C0},
        {"t105d24 Tranquility A-204", 0xAFA38C63},
        {"t105d25 Tranquility A-103", 0xD8A4BCF5},
        {"t105d26 Tranquility C-201", 0x41ADED4F},
        {"t105d27 Tranquility A-302", 0x36AADDD9},
        {"t105d28 Tranquility B-303", 0xA615C048},
        {"t105d29 Tranquility C-203", 0xD112F0DE},
        {"t105d30 Tranquility C-204", 0xB1D5793B},
        {"t105d31 Tranquility C-301", 0xC6D249AD},
        {"t105d32 Tranquility C-302", 0x5FDB1817},
        {"t105d33 Tranquility B-301 (Past)", 0x28DC2881},
        {"t105d34 Tranquility B-302 (Past)", 0xB6B8BD22},
        {"t105d35 Tranquility B-303 (Past)", 0xC1BF8DB4},
        {"t105d36 Tranquility B-304 (Past)", 0x58B6DC0E},
        {"t105d41 Nocturne Hospital 1F", 0x8993DF6A},
        {"t105d43 Nocturne Hospital 2F", 0x679DBE46},
        {"t105d45 Nocturne Hospital 3F", 0x8EFE1B73},
        {"t105d47 Hospital Basement", 0x60F07A5F},
        {"t105d48 Hospital Basement Lab", 0xF04F67CE},
        {"t106d11 Rugged Path", 0xE65184C1},
        {"t106d31 Gourd Pond Museum 1F", 0xD467E643},
        {"t106d32 Gourd Pond Museum 2F", 0x4D6EB7F9},
        {"t106d33 Gourd Pond Museum Vault", 0x3A69876F},
        {"t108d01 Shady Parking Lot", 0x1F95C2B3},
        {"t108d03 Junk Alley", 0xF19BA39F},
        {"t109d11 Hazy Manor", 0xBE329497},
        {"t109d13 Hazy Manor 2", 0x503CF5BB},
        {"t109d15 Lotus Lake", 0xB95F508E},
        {"t109d17 Cape of Evening Calm", 0x575131A2},
        {"t109d19 Forevermore Falls", 0xB0E91CA5},
        {"t109d21 Classroom", 0x951FC754},
        {"t109d23 Blind Alley", 0x7B11A678},
        {"t109d25 Requiem Riverside", 0x9272034D},
        {"t109d30 (unnamed)", 0xFB03C683},
        {"t121d01 Briny Grotto", 0x185DDE59},
        {"t121d03 Hidden Workshop", 0xF653BF75},
        {"t121d11 Seaside Cave", 0x0146EF18},
        {"t121d13 Dragon Heights", 0xEF488E34},
        {"t131d01 Rice Paddy Path", 0x253DF7E9},
        {"t131d03 Fullface Rock", 0xCB3396C5},
        {"t131d04 Cicada Canyon", 0x55570366},
        {"t131d05 Mt. Middleton Summit", 0x225033F0},
        {"t132d01 Alley off the Plaza", 0x37885807},
        {"t400d01 River Tootin Tributary", 0x8AEC7BEF},
        {"t400d11 Remote Relic", 0x93F74AAE},
        {"t400d13 Grumbler's Grotto Ent.", 0x7DF92B82},
        {"t400d15 Solitary Sanctuary", 0x949A8EB7},
        {"t400d17 Forest Islet", 0x7A94EF9B},
        {"t400d19 Secret Cavern", 0x9D2CC29C},
        {"t401d01 Garden Grill", 0x32501C8A},
        {"t401d03 Denton's Repairs", 0xDC5E7DA6},
        {"t401d21 Scrapyard", 0x00667E08},
        {"t401d99 (unnamed)", 0xED492FF1},
        {"t402d01 Hip & Hopping Alley", 0x20E5B364},
        {"t402d03 Warehouse No. 2", 0xCEEBD248},
        {"t402d21 Pearly Chamber of Whimsy", 0x12D3D1E6},
        {"t402d26 Whimsical Shopping 1", 0x8CB74445},
        {"t402d27 Whimsical Shopping 2", 0xFBB074D3},
        {"t402d28 Whimsical Shopping 3", 0x6B0F6942},
        {"t402d31 Whimsical Work 1", 0x0BC8E0A7},
        {"t402d32 Whimsical Work 2", 0x92C1B11D},
        {"t402d33 Whimsical Work 3", 0xE5C6818B},
        {"t402d36 Whimsical Quiz 1", 0x95AC7504},
        {"t402d37 Whimsical Quiz 2", 0xE2AB4592},
        {"t402d38 Whimsical Quiz 3", 0x72145803},
        {"t402d39 Whimsical Quiz Punishment", 0x05136895},
        {"t402d41 Whimsical Drilling 1", 0x44897660},
        {"t402d42 Whimsical Drilling 2", 0xDD8027DA},
        {"t402d43 Whimsical Drilling 3", 0xAA87174C},
        {"t402d44 Whimsical Drilling 4", 0x34E382EF},
        {"t403d01 Hotel Rear Byway", 0x9859D401},
        {"t403d11 Phantomart", 0x8142E540},
        {"t403d13 Phantomart Storeroom", 0x6F4C846C},
        {"t404d01 Hazeltine Underground", 0x058EECB8},
        {"t404d03 Hazeltine Entrance Hall", 0xEB808D94},
        {"t404d05 Hazeltine 1F Hallway", 0x02E328A1},
        {"t404d07 Hazeltine Dining Room", 0xECED498D},
        {"t404d09 Hazeltine Judgement Hall", 0x0B55648A},
        {"t404d11 Hazeltine Kitchen", 0x1C95DDF9},
        {"t404d13 Hazeltine Bathroom", 0xF29BBCD5},
        {"t404d15 Hazeltine 2F Hallway", 0x1BF819E0},
        {"t404d17 Hazeltine Archive", 0xF5F678CC},
        {"t404d19 Hazeltine Mansion", 0x124E55CB},
        {"t405d11 Gloombell Forest", 0xA429BA9C},
        {"t405d13 Mushroom Park", 0x4A27DBB0},
        {"t405d21 Grumbler's Grotto", 0x8F04E95F},
        {"t405d23 Grumbler's Grotto 2", 0x610A8873},
        {"t406d21 Meadowbrooke Farm (D)", 0x9DB146B1},
        {"t411d00 Hopper's Gorge", 0x783705AC},
        {"t411d10 Hopper's Gorge Condor Canyon", 0x612C34ED},
        {"t411d21 Zenlightenment 1st", 0x3D0657B8},
        {"t411d22 Zenlightenment 2nd", 0xA40F0602},
        {"t411d23 Zenlightenment 3rd", 0xD3083694},
        {"t411d24 Zenlightenment 4th", 0x4D6CA337},
        {"t411d25 Zenlightenment 5th", 0x3A6B93A1},
        {"t411d26 Zenlightenment 6th", 0xA362C21B},
        {"t411d27 Zenlightenment 7th", 0xD465F28D},
        {"t411d28 Zenlightenment 8th", 0x44DAEF1C},
        {"t411d29 Zenlightenment 9th", 0x33DDDF8A},
        {"t411d30 Zenlightenment Final", 0x531A566F},
        {"t411d41 Ascension Ground", 0x6B5CF03E},
        {"t501d01 Banquet Room", 0xF90CCF2F},
        {"t581d01 Cluvian Keyhole Ent.", 0xC97C84EE},
        {"t581d02 Tomb of Clupharaon Ent.", 0x5075D554},
        {"t581d03 Temple of Clusis Ent.", 0x2772E5C2},
        {"t581d04 Statue of Clunubis Ent.", 0xB9167061},
        {"t581d05 Pyramid of Clu Ent.", 0xCE1140F7},
        {"t581d06 Mt. Cluvimpus Ent.", 0x5718114D},
        {"t581d07 Cluphinx Ent.", 0x201F21DB},
        {"t581d08 Tower of Clu Ent.", 0xB0A03C4A},
        {"t581d09 Enma Palace Ent.", 0xC7A70CDC},
        {"t581d12 Sky Garden Ent.", 0x496EE415},
        {"t490d01 Suspicious Room", 0x87FC199E},
        {"t491d01 (unnamed)", 0x3F407EFB},
        {"t591d01 (unnamed)", 0xF41CAD5E},
        {"t592d01 (unnamed)", 0xE6A902B0},
        {"t592d02 (unnamed)", 0x7FA0530A},
        {"t592d03 (unnamed)", 0x08A7639C},
        {"t592d04 (unnamed)", 0x96C3F63F},
        {"t592d05 (unnamed)", 0xE1C4C6A9},
        {"t592d06 (unnamed)", 0x78CD9713},
        {"t592d07 (unnamed)", 0x0FCAA785},
        {"t592d08 (unnamed)", 0x9F75BA14},
        {"t592d10 (unnamed)", 0x88B50367},
        {"t592d11 (unnamed)", 0xFFB233F1},
        {"t592d12 (unnamed)", 0x66BB624B},

        // ---- 駅 ----
        {"t100s01 Springdale Central Stn", 0xC3485FA9},
        {"t100s02 Green St Stn (Sparkopolis)", 0x5A410E13},
        {"t100s03 Hibarly Hills Stn", 0x2D463E85},
        {"t100s04 Petal Peak Stn", 0xB322AB26},
        {"t100s05 Factory Row Stn", 0xC4259BB0},
        {"t100s06 Sweet Meadow Stn", 0x5D2CCA0A},
        {"t100s07 Fortune Place Stn", 0x2A2BFA9C},
        {"t100s08 Skybridge Stn", 0xBA94E70D},
        {"t100s09 Dreamer's Field Stn", 0xCD93D79B},
        {"t100s10 Ridgemont Stn", 0xAD545E7E},
        {"t100s11 Bayside Stn", 0xDA536EE8},
        {"t100s12 San Fantastico Stn", 0x435A3F52},
        {"t100s13 Greenfields Stn", 0x345D0FC4},
        {"t100s14 Temple Park Stn", 0xAA399A67},
        {"t100s15 Dingle Falls Stn", 0xDD3EAAF1},
        {"t100s16 Harrisville Stn", 0x4437FB4B},
        {"t100s17 Spring Stn", 0x3330CBDD},
        {"t100s18 Sunshine Stn", 0xA38FD64C},
        {"t100s19 Little Haven Stn", 0xD488E6DA},
        {"t100s20 Scarfit Downs Stn", 0x86790DBD},
        {"t100s21 Cherry Hill Stn", 0xF17E3D2B},
        {"t100s22 Whimsy Valley Stn", 0x68776C91},

        // ---- その他 ----
        {"testmap [test]", 0xEBB16BD2},
        {"t100t00 Train Interior 1", 0xB10079BA},
        {"t100t01 Train Interior 2", 0xC607492C},
        {"t100t02 Train Interior 3", 0x5F0E1896},
        {"t100t03 Train Interior 4", 0x28092800},
        {"t100t04 Train Interior 5", 0xB66DBDA3},
        {"t100t05 Train Interior 6", 0xC16A8D35},
        {"t100t06 Train Interior 7", 0x5863DC8F},
    };
    static const u32 g_mapCount = sizeof(g_maps) / sizeof(g_maps[0]);

    // ---- 状態 ----
    static u32         g_destId   = 0x91487158;
    static std::string g_destName = "t201g00 Old Springdale [unused]";
    static int         g_char     = 0;           // 0 = ケータ, 1 = イナホ
    static int         g_layout   = 1;           // 0 = 旧式9スロット(Old3DS用), 1 = 新式1か所(New3DS用・初期値), 2 = 手入力アドレス
    static u32         g_base     = 0x0B000000;
    static u32         g_custom   = 0x0B7266E4;
    static int         g_hotkey   = 5;           // 5 = A+B (ARコードと同じ)

    // Old3DS用ARコードにあった書き込み先(下位アドレス)。先頭だけがキャラごとに違う
    static const u32 LOW_KEITA[9] =
    {
        0x007266E4, 0x00728548, 0x0072DE58, 0x0072DEA8, 0x008EFB80,
        0x008F1C30, 0x008F4F8C, 0x00A909AC, 0x00C93B4C
    };
    static const u32 LOW_INAHO[9] =
    {
        0x007268F4, 0x00728548, 0x0072DE58, 0x0072DEA8, 0x008EFB80,
        0x008F1C30, 0x008F4F8C, 0x00A909AC, 0x00C93B4C
    };
    // New3DS用ARコードにあった書き込み先(0C3266E4 / 0C3268F4、オフセットなし)
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

    static std::string Dec(int v)
    {
        char b[16];
        snprintf(b, sizeof(b), "%d", v);
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

    // 選んでいるキャラの全スロットを、base 0B000000 と 08000000 の両方で表示(* = 一覧にあるマップID)
    static void ShowSlots(void)
    {
        const u32 *low = (g_char == 0) ? LOW_KEITA : LOW_INAHO;
        const u32 bases[2] = {0x0B000000, 0x08000000};
        for (int b = 0; b < 2; b++)
        {
            std::string t = "Base " + Hex8(bases[b]) + (g_char == 0 ? " Keita" : " Inaho") + " (* = known map ID)\n";
            for (int i = 0; i < 9; i++)
            {
                u32 v = 0;
                bool ok = Process::Read32(bases[b] + low[i], v);
                t += Hex8(low[i]) + ": " + (ok ? Hex8(v) : std::string("READ FAILED")) + ((ok && MapNameOf(v)) ? " *" : "") + "\n";
            }
            ShowInfo(t);
        }
        ShowInfo("New-style addresses:\n"
                 "Keita " + DescribeAddr(NEW_KEITA) + "\n"
                 "Inaho " + DescribeAddr(NEW_INAHO) + "\n"
                 "Custom " + DescribeAddr(g_custom));
    }

    // 選んでいる方式で行き先IDを書き込む。成功した書き込み数を返す
    static int DoWrite(u32 id, u32 &firstAddr)
    {
        int ok = 0;
        if (g_layout == 0)
        {
            const u32 *low = (g_char == 0) ? LOW_KEITA : LOW_INAHO;
            firstAddr = g_base + low[0];
            for (int i = 0; i < 9; i++)
                if (Process::Write32(g_base + low[i], id))
                    ok++;
        }
        else
        {
            firstAddr = (g_layout == 1) ? (g_char == 0 ? NEW_KEITA : NEW_INAHO) : g_custom;
            if (Process::Write32(firstAddr, id))
                ok = 1;
        }
        return ok;
    }

    static bool g_wasDown = false;

    // 毎フレーム呼ばれる。ホットキーを押している間だけ書き込む(ARの DD000000 と同じ)
    static void MapWarpFrame(MenuEntry *entry)
    {
        bool down = Controller::IsKeysDown(HotkeyMask(g_hotkey));
        if (down)
        {
            u32 first = 0;
            int ok = DoWrite(g_destId, first);
            if (!g_wasDown) // 押した瞬間だけ結果を表示
                OSD::Notify("write ok " + Dec(ok) + "  " + DescribeAddr(first));
        }
        g_wasDown = down;
    }

    static void SelectDestination(void)
    {
        std::vector<std::string> cats =
        {
            "Field (g)", "Interior (i)", "Dungeon etc. (d)", "Station (s)", "Other", "Hex input"
        };
        Keyboard kb("Category", cats);
        int c = kb.Open();
        if (c < 0)
            return;

        if (c == 5)
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

        static const char kinds[] = {'g', 'i', 'd', 's'};
        std::vector<u32>         idx;
        std::vector<std::string> names;
        for (u32 i = 0; i < g_mapCount; i++)
        {
            char t = g_maps[i].name[4];
            bool match;
            if (c < 4)
                match = (t == kinds[c]);
            else
                match = (t != 'g' && t != 'i' && t != 'd' && t != 's');
            if (match)
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
        static const char *layoutNames[] =
        {
            "Old-style 9 slots (low addr + base)", "New-style single address", "Custom address"
        };

        for (;;)
        {
            std::vector<std::string> opts =
            {
                "Destination: " + g_destName,
                std::string("Character: ") + (g_char == 0 ? "Keita" : "Inaho"),
                std::string("Layout: ") + layoutNames[g_layout],
                "Base offset: " + Hex8(g_base),
                std::string("Hotkey: ") + hotkeyNames[g_hotkey],
                "Check: show current values",
                "Set custom address (hex): " + Hex8(g_custom),
            };
            Keyboard kb("Map Warp (hold hotkey, then trigger a map load)", opts);
            int r = kb.Open();
            if (r < 0)
                return;

            if (r == 0)
                SelectDestination();
            else if (r == 1)
            {
                std::vector<std::string> chars = {"Keita", "Inaho"};
                Keyboard ck("Character", chars);
                int cr = ck.Open();
                if (cr >= 0) g_char = cr;
            }
            else if (r == 2)
            {
                std::vector<std::string> ls(layoutNames, layoutNames + 3);
                Keyboard lk("Layout", ls);
                int lr = lk.Open();
                if (lr >= 0) g_layout = lr;
            }
            else if (r == 3)
            {
                std::vector<std::string> bs = {"0x0B000000", "0x08000000", "Hex input"};
                Keyboard bk("Base offset (old-style slots)", bs);
                int br = bk.Open();
                if (br == 0) g_base = 0x0B000000;
                else if (br == 1) g_base = 0x08000000;
                else if (br == 2)
                {
                    u32 v = g_base;
                    Keyboard hex("Base offset (hex)");
                    hex.IsHexadecimal(true);
                    if (hex.Open(v, g_base) >= 0)
                        g_base = v;
                }
            }
            else if (r == 4)
            {
                std::vector<std::string> hk(hotkeyNames, hotkeyNames + 7);
                Keyboard hk2("Hotkey", hk);
                int hr = hk2.Open();
                if (hr >= 0) g_hotkey = hr;
            }
            else if (r == 5)
            {
                ShowSlots();
            }
            else if (r == 6)
            {
                u32 v = g_custom;
                Keyboard hex("Address (hex)");
                hex.IsHexadecimal(true);
                if (hex.Open(v, g_custom) >= 0)
                    g_custom = v;
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

        menu->Run();
        delete menu;
        return 0;
    }
}
