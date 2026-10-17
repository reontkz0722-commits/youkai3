// 妖怪ウォッチ2 真打 ツール (CTRPluginFramework) — Luma3DS 3GX プラグイン
// タイトルID: 0004000000155100
//
// 機能:
//   Movement : 座標移動(B+十字キー) / ムーンジャンプ(ZR上・ZL下) / 空中歩行 / 低速落下 / 壁抜け /
//              スタミナ無限 / スピード変更 / 座標表示
//   Map Warp : 全マップ(315件)から行き先を選んで移動。マップIDの枠のアドレスは「Find map slot」で探す
//
// アドレスの出どころ: Eve氏の yw2txt (ARコード集) と、既存の真打用3gx(Yo-kaiWatch2.3gx)。
// ※ マップワープの枠のアドレスだけは、ARコードが見つかっていないので、実機で探して設定する。

#include <3ds.h>
#include "csvc.h"
#include <CTRPluginFramework.hpp>
#include <algorithm>
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

    // 名前の先頭がファイル名: t***g=フィールド i=屋内 d=ダンジョン b=戦闘 s=駅 e***=イベント
    static const MapInfo g_maps[] =
    {
        {"e104d331", 0x943A982C},
        {"e206g003", 0x0AC2F289},
        {"e250g001", 0x3B41F7B6},
        {"e270g001", 0x768956BD},
        {"e270g003", 0x98873791},
        {"e270g005", 0x71E492A4},
        {"e270g007", 0x9FEAF388},
        {"t001b12", 0x2D5E42D5},
        {"t001d01 ごくらく温泉 入場口", 0xA9C15E9C},
        {"t001d02 ゲラゲランド 入場口", 0x30C80F26},
        {"t001d03 きもだ飯 入場口", 0x47CF3FB0},
        {"t001d05 ごくらく温泉", 0xAEAC9A85},
        {"t001d06 ゲラゲランド", 0x37A5CB3F},
        {"t001d07 きもだ飯", 0x40A2FBA9},
        {"t001d08 怪舞伎座 スーパーステージ", 0xD01DE638},
        {"t001d09 怪舞伎座 奈落", 0xA71AD6AE},
        {"t001d41 妖魔界 世界の境目", 0xCDAD9B98},
        {"t001d42 妖魔界 えっけん回廊", 0x54A4CA22},
        {"t001d43 妖魔界 うそつき山", 0x23A3FAB4},
        {"t001d44 妖魔界 あらくれ街道", 0xBDC76F17},
        {"t001d45 妖魔界 はらぺこ峠", 0xCAC05F81},
        {"t001s00 Unknown (Hexpress?) S..", 0xC7AFDBFF},
        {"t001s01 ごくらく温泉", 0xB0A8EB69},
        {"t001s02 ゲラゲランド", 0x29A1BAD3},
        {"t001s03 きもだ飯", 0x5EA68A45},
        {"t001t01 妖魔特急 機関室", 0xB5E7FDEC},
        {"t001t03 妖魔特急 普通車", 0x5BE99CC0},
        {"t001t04 妖魔特急 食堂車", 0xC58D0963},
        {"t002s01 満福おたふく", 0xA21D4487},
        {"t002t03 ふくふく超特急 普通車", 0x495C332E},
        {"t002t04 ふくふく超特急 食堂車", 0xD738A68D},
        {"t100d00 桜町地下水道", 0xAD26DACA},
        {"t100s01 桜中央", 0xC3485FA9},
        {"t100s02 アオバ", 0x5A410E13},
        {"t100s03 ひばり台", 0x2D463E85},
        {"t100s04 はなやま", 0xB322AB26},
        {"t100s05 いわくり", 0xC4259BB0},
        {"t100s06 かすが野", 0x5D2CCA0A},
        {"t100s07 福ノ宮", 0x2A2BFA9C},
        {"t100s08 はしぶね", 0xBA94E70D},
        {"t100s09 ねぼとけ", 0xCD93D79B},
        {"t100s10 やまなか", 0xAD545E7E},
        {"t100s11 きしべ", 0xDA536EE8},
        {"t100s12 ナギサキ", 0x435A3F52},
        {"t100s13 たぞの", 0x345D0FC4},
        {"t100s14 おさらぎ", 0xAA399A67},
        {"t100s15 たにあい", 0xDD3EAAF1},
        {"t100s16 ケマモト", 0x4437FB4B},
        {"t100s17 さくらぎ", 0x3330CBDD},
        {"t100s18 こひなた", 0xA38FD64C},
        {"t100s19 すんどめ", 0xD488E6DA},
        {"t100s20 おかわり", 0x86790DBD},
        {"t100s21 さくら山", 0xF17E3D2B},
        {"t100s22 むこう谷", 0x68776C91},
        {"t100t00 Train Interior #1 (De..", 0xB10079BA},
        {"t100t01 Train Interior #2", 0xC607492C},
        {"t100t02 Train Interior #3", 0x5F0E1896},
        {"t100t03 Train Interior #4", 0x28092800},
        {"t100t04 Train Interior #5", 0xB66DBDA3},
        {"t100t05 Train Interior #6", 0xC16A8D35},
        {"t100t06 Train Interior #7", 0x5863DC8F},
        {"t100t07 Train Interior #8", 0x2F64EC19},
        {"t101d01 怪しい路地裏", 0x629D8D39},
        {"t101d02 さびしげな水路", 0xFB94DC83},
        {"t101d02c 金ピカ都市高 外回り", 0x72F39929},
        {"t101d03 ネコの通り道", 0x8C93EC15},
        {"t101d03c あやかし通り ２号線", 0x6BE8A868},
        {"t101d05 ひとなし路地", 0x65F04920},
        {"t101d05c 金ピカ都市高 東行き", 0x3DB20FEE},
        {"t101g00 さくら住宅街", 0x17DC03F6},
        {"t101i01 天野家 １階", 0x6A451E6A},
        {"t101i02 天野家 ２階", 0xF34C4FD0},
        {"t101i03 木霊家 １階", 0x844B7F46},
        {"t101i04 木霊家 ２階", 0x1A2FEAE5},
        {"t101i21 アッカンベーカリー", 0x58737CE8},
        {"t101i23 ヨロズマート さんかく通り店", 0xB67D1DC4},
        {"t101i25 桜町公民館", 0x5F1EB8F1},
        {"t101i27 こぶた銀行", 0xB110D9DD},
        {"t101i29 こやぎ郵便", 0x56A8F4DA},
        {"t101i31 ジャングルハンター", 0x41684DA9},
        {"t101i35 おもいで屋", 0x460589B0},
        {"t101i51 さくら第一小学校 南校舎１階", 0x1732EA2F},
        {"t101i52 さくら第一小学校 北校舎１階", 0x8E3BBB95},
        {"t101i53 さくら第一小学校 ２階", 0xF93C8B03},
        {"t101i55 さくら第一小学校 ３階", 0x105F2E36},
        {"t101i58 さくら第一小学校 屋上", 0x6EEE528B},
        {"t101i59 さくら第一小学校 体育館", 0x19E9621D},
        {"t102d01 おおもり山 登山道", 0x702822D7},
        {"t102d01c 真の元祖道 出口", 0x1E7EB03A},
        {"t102d02 おおもり山 山頂", 0xE921736D},
        {"t102d03 ジャンボスライダー", 0x9E2643FB},
        {"t102d03c 真の本家道 出口", 0x2C48D2B8},
        {"t102d31 おおもり山の廃トンネル 西の空洞", 0x5B057114},
        {"t102d32 おおもり山の廃トンネル 東の空洞", 0xC20C20AE},
        {"t102g00 おおもり山", 0x0569AC18},
        {"t102i01 廃屋", 0x78F0B184},
        {"t102i02 廃屋", 0xE1F9E03E},
        {"t102i21 滝の裏の祠", 0x4AC6D306},
        {"t103d01 こっそり空地", 0xC89445B2},
        {"t103d03 ないしょの横道", 0x269A249E},
        {"t103d03c あやかし通り 裏道", 0x1128FB08},
        {"t103d11 ひみつの抜け道", 0xD18F74F3},
        {"t103d11c あやかし通り ６号線", 0x22DCF3BD},
        {"t103d31 竹林のおんぼろ屋敷 母屋", 0xE3B91671},
        {"t103d33 竹林のおんぼろ屋敷 離れ", 0x0DB7775D},
        {"t103d35 竹林のおんぼろ屋敷 母屋の屋根裏", 0xE4D4D268},
        {"t103d36 竹林のおんぼろ屋敷 離れの屋根裏", 0x7DDD83D2},
        {"t103d41 ムゲン地獄 第１階層", 0xACF880B6},
        {"t103d42 ムゲン地獄 第２階層", 0x35F1D10C},
        {"t103d43 ムゲン地獄 第３階層", 0x42F6E19A},
        {"t103d44 ムゲン地獄 第４階層", 0xDC927439},
        {"t103d45 ムゲン地獄 第５階層", 0xAB9544AF},
        {"t103d46 ムゲン地獄 第６階層", 0x329C1515},
        {"t103d47 ムゲン地獄 第７階層", 0x459B2583},
        {"t103d48 ムゲン地獄 第８階層", 0xD5243812},
        {"t103d49 ムゲン地獄 ウバステの間", 0xA2230884},
        {"t103d50 アミダ極楽 第１階層", 0xC2E48161},
        {"t103d51 アミダ極楽 第２階層", 0xB5E3B1F7},
        {"t103d52 アミダ極楽 第３階層", 0x2CEAE04D},
        {"t103d53 アミダ極楽 第４階層", 0x5BEDD0DB},
        {"t103d54 アミダ極楽 第５階層", 0xC5894578},
        {"t103d55 アミダ極楽 第６階層", 0xB28E75EE},
        {"t103d56 ムゲン地獄 ウバステの間", 0x2B872454},
        {"t103g00 団々坂", 0xBDD5CB7D},
        {"t103i01 熊島家 １階", 0xC04CD6E1},
        {"t103i02 熊島家 ２階", 0x5945875B},
        {"t103i03 熊島家 ３階", 0x2E42B7CD},
        {"t103i21 時計のチョーシ堂", 0xF27AB463},
        {"t103i23 かわしま商店", 0x1C74D54F},
        {"t103i25 ヨロズマート 団々坂店", 0xF517707A},
        {"t103i27", 0x1B191156},
        {"t103i28", 0x8BA60CC7},
        {"t103i31 正天寺", 0xEB618522},
        {"t103i33 ひがん山トンネル", 0x056FE40E},
        {"t103i35 冥心漢方", 0xEC0C413B},
        {"t103i37 さくらの湯 ロビー", 0x02022017},
        {"t103i38 さくらの湯 <CN>男湯</C>", 0x92BD3D86},
        {"t103i39 さくらの湯 <CR>女湯</C>", 0xE5BA0D10},
        {"t103i51 鳥飼家 大広間", 0xBD3B22A4},
        {"t103i53 鳥飼家 隠し部屋", 0x53354388},
        {"t103i60 さすらい荘 １０１号室", 0xE11141F1},
        {"t103i61 さすらい荘 １０２号室", 0x96167167},
        {"t103i62 さすらい荘 １０３号室", 0x0F1F20DD},
        {"t103i63 さすらい荘 １０４号室", 0x7818104B},
        {"t103i64 さすらい荘 １０５号室", 0xE67C85E8},
        {"t103i65 さすらい荘 ２０１号室", 0x917BB57E},
        {"t103i66 さすらい荘 ２０２号室", 0x0872E4C4},
        {"t103i67 さすらい荘 ２０３号室", 0x7F75D452},
        {"t103i68 さすらい荘 ２０４号室", 0xEFCAC9C3},
        {"t103i69 さすらい荘 ＶＩＰルーム", 0x98CDF955},
        {"t104d01 塾への近道", 0x55437D0B},
        {"t104d03 ゆきおんなの裏道", 0xBB4D1C27},
        {"t104d03c 金ピカ都市高 西行き", 0xA3082718},
        {"t104d05 うんぱん通路", 0x522EB912},
        {"t104d11 さくらビジネスガーデンビル ４階", 0x4C584C4A},
        {"t104d13 さくらビジネスガーデンビル １３階", 0xA2562D66},
        {"t104d31", 0x7E6E2EC8},
        {"t104d32", 0xE7677F72},
        {"t104d33 工事現場 ３階", 0x90604FE4},
        {"t104g00 さくら中央シティ", 0x2002F3C4},
        {"t104i21 しおかぜトンネル", 0x6FAD8CDA},
        {"t104i23 ゆきおんな", 0x81A3EDF6},
        {"t104i25 ピコピコランド", 0x68C048C3},
        {"t104i27 モグモグバーガー", 0x86CE29EF},
        {"t104i29 福北病院 １Ｆ", 0x617604E8},
        {"t104i30 福北病院 ２Ｆ", 0x01B18D0D},
        {"t104i31 はるこい学習塾", 0x76B6BD9B},
        {"t104i33 ヨロズマート さくら中央シティ店", 0x98B8DCB7},
        {"t104i35 カフェ・オ・シャレンヌ", 0x71DB7982},
        {"t104i37 さくらスポーツクラブ １階", 0x9FD518AE},
        {"t104i38 さくらスポーツクラブ ２階", 0x0F6A053F},
        {"t104i39 さくらスポーツクラブ ３階", 0x786D35A9},
        {"t104i41 カレーハウス ナマステ", 0x39F72B5C},
        {"t104i51 さくらビジネスガーデンビル １階", 0x20EC1A1D},
        {"t104i53 さくらビジネスガーデンビル ７階", 0xCEE27B31},
        {"t104i61 桜中央駅", 0x0BC149DE},
        {"t105d01 商店街の細道", 0xEDFF1A6E},
        {"t105d01c あやかし通り ５号線", 0xAC5E6C2A},
        {"t105d11 あんのん団地 Ａ−１０１号室", 0xF4E42B2F},
        {"t105d12 あんのん団地 Ａ−２０１号室", 0x6DED7A95},
        {"t105d13 あんのん団地 Ａ−２０３号室", 0x1AEA4A03},
        {"t105d14 あんのん団地 Ｂ−１０２号室", 0x848EDFA0},
        {"t105d15 あんのん団地 Ｂ−２０１号室", 0xF389EF36},
        {"t105d16 あんのん団地 Ｂ−２０４号室", 0x6A80BE8C},
        {"t105d17 あんのん団地 Ｂ−３０１号室", 0x1D878E1A},
        {"t105d18 あんのん団地 Ｃ−１０１号室", 0x8D38938B},
        {"t105d19 あんのん団地 Ｃ−３０２号室", 0xFA3FA31D},
        {"t105d20 あんのん団地 Ｃ−２０２号室", 0xA8CE487A},
        {"t105d21 あんのん団地 Ｃ−２０４号室", 0xDFC978EC},
        {"t105d22 あんのん団地 Ｃ−３０３号室", 0x46C02956},
        {"t105d23 あんのん団地 Ａ−２０２号室", 0x31C719C0},
        {"t105d24 あんのん団地 Ｂ−１０４号室", 0xAFA38C63},
        {"t105d25 あんのん団地 Ｃ−２０３号室", 0xD8A4BCF5},
        {"t105d26 あんのん団地 Ａ−３０２号室", 0x41ADED4F},
        {"t105d41 かげむら医院 １階", 0x8993DF6A},
        {"t105d43 かげむら医院 ２階", 0x679DBE46},
        {"t105d45 かげむら医院 ３階", 0x8EFE1B73},
        {"t105d47 かげむら医院 地下階段", 0x60F07A5F},
        {"t105d48 かげむら医院 地下実験場", 0xF04F67CE},
        {"t105g00 おつかい横丁", 0x98BE94A1},
        {"t105i20 桜町フラワーロード", 0xA016DB29},
        {"t105i21 どっこい書店", 0xD711EBBF},
        {"t105i23 北風ラーメン", 0x391F8A93},
        {"t105i25 ヨロズマート おつかい横丁店", 0xD07C2FA6},
        {"t105i27 タイヨー軒", 0x3E724E8A},
        {"t105i29 ワクワクポケット", 0xD9CA638D},
        {"t105i31 コインランドリー メリー", 0xCE0ADAFE},
        {"t105i33 カリスマスタイル", 0x2004BBD2},
        {"t105i35 めっけもん", 0xC9671EE7},
        {"t105i37 お花見寿司", 0x27697FCB},
        {"t106d11 もののけ道", 0xE65184C1},
        {"t106d11c あやかし通り 側道", 0xEA3C7CCD},
        {"t106d31 ひょうたん池博物館 １階", 0xD467E643},
        {"t106d32 ひょうたん池博物館 ２階", 0x4D6EB7F9},
        {"t106d33 ひょうたん池博物館 資料保管庫", 0x3A69876F},
        {"t106d41 ゲートの間", 0x9B267084},
        {"t106d43 創造主の間", 0x752811A8},
        {"t106d45 缶けりの間 １階", 0x9C4BB49D},
        {"t106d46 缶けりの間 ２階", 0x0542E527},
        {"t106d47 缶けりの間 ３階", 0x7245D5B1},
        {"t106d49 缶けりの間 出口", 0x95FDF8B6},
        {"t106d51 信号の間 １階", 0x823D41C5},
        {"t106d52 信号の間 ２階", 0x1B34107F},
        {"t106d53 信号の間 ３階", 0x6C3320E9},
        {"t106d54 信号の間 出口", 0xF257B54A},
        {"t106d56 おすべりの間 １階", 0x1C59D466},
        {"t106d57 おすべりの間 ２階", 0x6B5EE4F0},
        {"t106d58 おすべりの間 ３階", 0xFBE1F961},
        {"t106d59 おすべりの間 出口", 0x8CE6C9F7},
        {"t106d61 ナゾナゾの間 １階", 0xA9101206},
        {"t106d62 ナゾナゾの間 ２階", 0x301943BC},
        {"t106d63 ナゾナゾの間 ３階", 0x471E732A},
        {"t106d64 ナゾナゾの間 出口", 0xD97AE689},
        {"t106g00 そよ風ヒルズ", 0x8A0B3B4F},
        {"t106i01 今田家 １階", 0xF79226D3},
        {"t106i02 今田家 ２階", 0x6E9B7769},
        {"t106i05 蔵岩家", 0xF0FFE2CA},
        {"t106i07 エミちゃんの家 １階", 0x1EF183E6},
        {"t106i09 エミちゃんの家 ２階", 0xF949AEE1},
        {"t106i21 ヨロズマート そよ風ヒルズ店", 0xC5A44451},
        {"t106i23 トロフィールーム", 0x2BAA257D},
        {"t106i51 リリィガーデン 地下駐車場", 0x8AE5D296},
        {"t106i52 リリィガーデン エントランス", 0x13EC832C},
        {"t107g00 さくらＥＸツリー", 0x32B75C2A},
        {"t107i01 さくらＥＸツリー エントランス", 0x4F2E41B6},
        {"t107i02 さくらＥＸツリー エレベーター", 0xD627100C},
        {"t107i03 さくらＥＸツリー 展望台", 0xA120209A},
        {"t121d01 潮騒の岩屋", 0x185DDE59},
        {"t121d03 裏の作業場", 0xF653BF75},
        {"t121d03c あやかし通り 本線", 0x26200963},
        {"t121d11 海辺の洞穴", 0x0146EF18},
        {"t121d21 あやかし通り １号線", 0x2A6BBCDB},
        {"t121d23 あやかし通り ４号線", 0xC465DDF7},
        {"t121d25 あやかし通り ランプウェイ", 0x2D0678C2},
        {"t121d27 ゴーゴー ジャンクション", 0xC30819EE},
        {"t121d27a ゴーゴー ジャンクション", 0xAFC67925},
        {"t121d27b ゴーゴー ジャンクション", 0x36CF289F},
        {"t121d29 あやかし通り 終点", 0x24B034E9},
        {"t121d31 金ピカ都市高 入口", 0x33708D9A},
        {"t121d33 金ピカ都市高 インターチェンジ", 0xDD7EECB6},
        {"t121d35 金ピカ都市高 終点", 0x341D4983},
        {"t121d41", 0x7C311B5D},
        {"t121g00 ナギサキ", 0x6D1C5096},
        {"t121i01 さざなみ集会所", 0x10854D0A},
        {"t121i03 こじま商店", 0xFE8B2C26},
        {"t121i10 廃屋", 0x7E994CDD},
        {"t121i20 海上", 0x55B41F1E},
        {"t131d01 棚田の跡地", 0x253DF7E9},
        {"t131d01c 金ピカ都市高 内回り", 0xDF4AB844},
        {"t131d02 ツチノコの里", 0xBC34A653},
        {"t131d03 正面岩", 0xCB3396C5},
        {"t131d03c 真の元祖道 入口", 0xED7CDAC6},
        {"t131d04 かわせみ渓谷", 0x55570366},
        {"t131d05 キウチ山 山頂", 0x225033F0},
        {"t131d20 えんえんトンネル", 0x600CA5FD},
        {"t131d32 むこうがわ", 0x9719F590},
        {"t131d33 むこうがわ", 0xE01EC506},
        {"t131d34 むこうがわ", 0x7E7A50A5},
        {"t131d35 むこうがわ", 0x097D6033},
        {"t131d36 暗黒の間", 0x90743189},
        {"t131g00 ケマモト村", 0x507C7926},
        {"t131i01 おばあちゃんの家", 0x2DE564BA},
        {"t131i03 ケマモト分校", 0xC3EB0596},
        {"t131i05 入山商店", 0x2A88A0A3},
        {"t132d01 駅前の古い民家", 0x37885807},
        {"t132d01c あやかし通り ３号線", 0x98EAC294},
        {"t132g00 ケマモト駅 駅前", 0x42C9D6C8},
        {"t200b41 平釜平原", 0x3C542144},
        {"t200d00 平釜平原", 0x2BB2A864},
        {"t201d01 お稲荷さん通り", 0xE409FF97},
        {"t201d01c 真の本家道 入口", 0x6836D077},
        {"t201d03 中井戸通り", 0x0A079EBB},
        {"t201d11 桜町鉄工所", 0xFD12CED6},
        {"t201d21 夕陽ヶ丘工場 正門", 0xD63F9D15},
        {"t201d23 夕陽ヶ丘工場 第一炉", 0x3831FC39},
        {"t201g00 桜町", 0x91487158},
        {"t201i01 正天寺", 0xECD16CC4},
        {"t201i03 超視堂", 0x02DF0DE8},
        {"t201i10 花道商店街", 0x82CD6D13},
        {"t201i20 ひがん山トンネル", 0xA9E03ED0},
        {"t202g00 おおもり山 おおもり神社", 0x83FDDEB6},
        {"t206g00 ひょうたん池", 0x0C9F49E1},
        {"t231d01 棚田", 0xA3A98547},
        {"t231d03 正面岩", 0x4DA7E46B},
        {"t231d04 かわせみ渓谷", 0xD3C371C8},
        {"t231d05 キウチ山 山頂", 0xA4C4415E},
        {"t231g00 ケマモト村", 0xD6E80B88},
        {"t231i01 しのびの森", 0xAB711614},
        {"t231i03 秘密基地", 0x457F7738},
        {"t231i05 昔の家", 0xAC1CD20D},
        {"t232g00 ケマモト駅 駅前", 0xC45DA466},
        {"t301g00 佐和山城 城下町", 0x5A14A2FD},
        {"t302g00 佐和山城 城内", 0x48A10D13},
        {"t302i01 大広間", 0x3538108F},
        {"t303g00 関ヶ原 石田軍本陣", 0xF01D6A76},
        {"t121d05 (未登録・データのみ)", 0x1F301A40},
        {"t201i05 (未登録・データのみ)", 0xEBBCA8DD},
        {"t107i80 (未登録・データのみ)", 0xF0F0FB28},
        {"t201b23 (未登録・データのみ)", 0x3CBC808B},
        {"t106b37 (未登録・データのみ)", 0x39893FC4},
        {"t002s00 (未登録・データのみ)", 0xD51A7411},
        {"t100s00 (未登録・データのみ)", 0xB44F6F3F},
        {"t106b32 (未登録・データのみ)", 0x49E3CB4B},
        {"t106b35 (未登録・データのみ)", 0xD7875EE8},
        {"t106b39 (未登録・データのみ)", 0xDE3112C3},
        {"t106b41 (未登録・データのみ)", 0x9FAB0C36},
        {"t106b43 (未登録・データのみ)", 0x71A56D1A},
        {"t120i01 (未登録・データのみ)", 0xA8392A6F},
        {"t121w011 (未登録・データのみ)", 0xD3DFA348},
    };
    static const u32 g_mapCount = sizeof(g_maps) / sizeof(g_maps[0]);

    // ================= 共通の道具 =================
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

    // UTF-8 を文字(コードポイント)単位で n 文字までに切る(画面で見切れないように)
    static std::string TruncU8(const std::string &s, size_t n)
    {
        size_t i = 0, cnt = 0;
        while (i < s.size() && cnt < n)
        {
            unsigned char c = (unsigned char)s[i];
            size_t len = (c < 0x80) ? 1 : (c >= 0xF0 ? 4 : (c >= 0xE0 ? 3 : 2));
            i += len;
            cnt++;
        }
        if (i >= s.size())
            return s;
        return s.substr(0, i) + "..";
    }

    // ラベルの先頭(ファイル名 t101g00 など)だけを取り出す。OSD/通知は ASCII だけにして文字化けを避ける
    static std::string CodeOf(const char *label)
    {
        std::string t = label ? label : "";
        size_t sp = t.find(' ');
        return (sp == std::string::npos) ? t : t.substr(0, sp);
    }

    static std::string DescribeAddr(u32 addr, bool ascii = false)
    {
        u32 v = 0;
        if (!Process::Read32(addr, v))
            return Hex8(addr) + ": 読み取り失敗";
        const char *n = MapNameOf(v);
        std::string name = n ? (ascii ? CodeOf(n) : std::string(n)) : std::string();
        return Hex8(addr) + ": " + Hex8(v) + (n ? " = " + name : std::string(" (一覧にない)"));
    }

    static void ShowInfo(const std::string &text)
    {
        std::vector<std::string> ok = {"OK"};
        Keyboard k(text, ok);
        k.Open();
    }

    // Controller::IsKeyDown は Key 型を受け取るので、数値のキーマスクを変換して呼ぶ
    static bool KeyDown(u32 mask)
    {
        return Controller::IsKeyDown((Key)mask);
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

    // ================= アドレス(ARコード由来・Old3DS基準) =================
    // Old3DS用の 08xxxxxx を基準にして、環境ごとに次のようにずらす(変換ツールの規則):
    //   New3DS : +0x03C00000      Citra/Azahar : -0x00001000      Old3DS : そのまま
    // コード領域(00xxxxxx)は同じ。レイアウトは自動判定(枠のマップIDが読める場所)か、手動で選ぶ。
    static int g_layoutMode = 0; // 0=自動 1=New3DS 2=Old3DS 3=Citra

    // 実際に使うレイアウト: 0=Old 1=New 2=Citra
    static int CurLayout(void)
    {
        if (g_layoutMode == 1) return 1;
        if (g_layoutMode == 2) return 0;
        if (g_layoutMode == 3) return 2;
        static int cache = -1; // 判定できたら固定する
        if (cache < 0)
        {
            u32 v = 0;
            if (Process::Read32(0x0C327284, v) && MapNameOf(v))
                cache = 1;
            else if (Process::Read32(0x08727284, v) && MapNameOf(v))
                cache = 0;
            else if (Process::Read32(0x08726284, v) && MapNameOf(v))
                cache = 2;
            else
                return 0; // まだ判定できない間は Old3DS 配置として扱う(真打は実機でもこの配置が普通)
        }
        return cache;
    }

    static const char *LayoutName(void)
    {
        switch (CurLayout())
        {
            case 0:  return "Old3DS";
            case 2:  return "Citra";
            default: return "New3DS";
        }
    }

    static u32 Adj(u32 oldAddr)
    {
        u32 top = oldAddr >> 24;
        if (top < 0x08 || top > 0x0C)
            return oldAddr; // コード領域などはそのまま
        int l = CurLayout();
        if (l == 1) return oldAddr + 0x03C00000;
        if (l == 2) return oldAddr - 0x1000;
        return oldAddr;
    }

    #define PLAYER_X_ADDR  (Adj(0x08852E10))
    #define PLAYER_Y_ADDR  (Adj(0x08852E14)) /* 高さ */
    #define PLAYER_Z_ADDR  (Adj(0x08852E18))
    #define AIR_WALK_ADDR  (Adj(0x08001DC0)) /* 空中歩行 : 0 を書く(txt) */
    #define SLOW_FALL_ADDR (Adj(0x0883A4C4)) /* 低速落下(txt) / 空中歩行可能(3gx) : 1 を書く(1バイト) */
    #define STAMINA_ADDR   (Adj(0x08001F2C)) /* スタミナ(float)。1.0 で無限 */
    #define SPEED_ADDR     (Adj(0x08001EFC)) /* 速度(float) */
    #define MAP_SLOT_DEFAULT (Adj(0x08727284)) /* ワープコードの枠: D3000000 08000000 + 00727284 */
    static const u32 WALL_ADDR1 = 0x00267F20; // 壁抜けのコードパッチ(コード領域なので共通)
    static const u32 WALL_ADDR2 = 0x00267F68;

    // ================= Movement =================
    static bool g_coordMove = false;
    static float g_coordStep = 30.0f;
    static bool g_moonJump = false;
    static float g_moonStep = 20.0f;
    static bool g_airWalk = false;
    static bool g_slowFall = false;
    static bool g_stamina = false;
    static int  g_speedMode = 0; // 0=変更しない 1=普通 2=速い 3=遅い
    static bool g_wall = false;
    static bool g_showCoords = false;

    static void SetWall(bool on)
    {
        g_wall = on;
        Process::Patch(WALL_ADDR1, on ? 0xE3A00001 : 0xE3A00000);
        Process::Patch(WALL_ADDR2, on ? 0xE3A01001 : 0xE3A01000);
    }

    static void MoveFrame(MenuEntry *entry)
    {
        if (g_coordMove && KeyDown(0x02)) // B + 十字キー
        {
            if (KeyDown(0x10)) AddToFloat(PLAYER_X_ADDR,  g_coordStep); // 右
            if (KeyDown(0x20)) AddToFloat(PLAYER_X_ADDR, -g_coordStep); // 左
            if (KeyDown(0x80)) AddToFloat(PLAYER_Z_ADDR,  g_coordStep); // 下
            if (KeyDown(0x40)) AddToFloat(PLAYER_Z_ADDR, -g_coordStep); // 上
        }
        if (g_moonJump)
        {
            if (KeyDown(0x8000))      AddToFloat(PLAYER_Y_ADDR,  g_moonStep); // ZR 上昇
            else if (KeyDown(0x4000)) AddToFloat(PLAYER_Y_ADDR, -g_moonStep); // ZL 下降
        }
        if (g_airWalk)
            Process::Write8(SLOW_FALL_ADDR, 1);          // 空中歩行(Yota版: 0883A4C4 に 1)
        if (g_stamina)
        {
            Process::Write32(AIR_WALK_ADDR, 0);          // スタミナ無限(Yota版: 08001DC0 に 0 = 疲れない状態)
            Process::Write32(STAMINA_ADDR, 0x3F800000);  // ダッシュゲージも満タンに保つ(ARコード集)
        }
        if (g_speedMode == 1)      Process::Write32(SPEED_ADDR, 0x3F800000);
        else if (g_speedMode == 2) Process::Write32(SPEED_ADDR, 0x5EFFFFFF);
        else if (g_speedMode == 3) Process::Write32(SPEED_ADDR, 0x3F000000);
    }

    static void MoveMenu(MenuEntry *entry)
    {
        static const char *speedNames[] = {"変更なし", "普通", "速い", "遅い"};
        for (;;)
        {
            std::vector<std::string> opts =
            {
                std::string("座標移動 B+十字: ") + (g_coordMove ? "ON" : "OFF"),
                "座標移動の速さ: " + Dec((u32)g_coordStep),
                std::string("ムーンジャンプ ZR/ZL: ") + (g_moonJump ? "ON" : "OFF"),
                "ムーンジャンプの速さ: " + Dec((u32)g_moonStep),
                std::string("空中歩行: ") + (g_airWalk ? "ON" : "OFF"),
                std::string("壁抜け: ") + (g_wall ? "ON" : "OFF"),
                std::string("スタミナ無限: ") + (g_stamina ? "ON" : "OFF"),
                std::string("スピード: ") + speedNames[g_speedMode],
                std::string("座標を画面に表示: ") + (g_showCoords ? "ON" : "OFF"),
            };
            Keyboard kb("移動系", opts);
            int r = kb.Open();
            if (r < 0)
                return;

            if (r == 0) g_coordMove = !g_coordMove;
            else if (r == 1)
            {
                u32 v = (u32)g_coordStep;
                Keyboard sk("座標移動の速さ(初期30)");
                if (sk.Open(v, (u32)g_coordStep) >= 0 && v > 0 && v < 1000) g_coordStep = (float)v;
            }
            else if (r == 2) g_moonJump = !g_moonJump;
            else if (r == 3)
            {
                u32 v = (u32)g_moonStep;
                Keyboard sk("ムーンジャンプの速さ(初期20)");
                if (sk.Open(v, (u32)g_moonStep) >= 0 && v > 0 && v < 1000) g_moonStep = (float)v;
            }
            else if (r == 4) g_airWalk = !g_airWalk;
            else if (r == 5) SetWall(!g_wall);
            else if (r == 6) g_stamina = !g_stamina;
            else if (r == 7)
            {
                std::vector<std::string> sn(speedNames, speedNames + 4);
                Keyboard sk("スピード", sn);
                int sr = sk.Open();
                if (sr >= 0) g_speedMode = sr;
            }
            else if (r == 8) g_showCoords = !g_showCoords;
        }
    }

    // ================= マップワープ =================
    static u32         g_destId   = 0;
    static std::string g_destName = "(not selected)";
    static u32         g_slot     = 0;     // 現在のマップIDの枠のアドレス。0 なら既定(ワープコードの 00727284)
    static bool        g_trigOn   = true;  // Yを巨大な負数にしてエリア再読み込みを起こす
    static u32         g_trigAddr = 0;     // 0 なら既定(Y座標)
    static int         g_hotkey   = 5;     // 5 = A+B

    static u32 SlotAddr(void) { return g_slot ? g_slot : MAP_SLOT_DEFAULT; }
    static u32 TrigAddr(void) { return g_trigAddr ? g_trigAddr : PLAYER_Y_ADDR; }

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

    static void DoTrigger(void)
    {
        u32 raw = 0;
        u32 ta = TrigAddr();
        if (!Process::Read32(ta, raw))
            return;
        float f;
        memcpy(&f, &raw, sizeof(f));
        u32 addBits = 0xDD000000; // 約 -5.8e17
        float a;
        memcpy(&a, &addBits, sizeof(a));
        f += a;
        memcpy(&raw, &f, sizeof(raw));
        Process::Write32(ta, raw);
    }

    static bool g_wasDown = false;
    static void MapWarpFrame(MenuEntry *entry)
    {
        bool down = Controller::IsKeysDown(HotkeyMask(g_hotkey));
        if (down)
        {
            if (g_destId == 0)
            {
                if (!g_wasDown)
                    OSD::Notify("マップワープ: 先に行き先を選んでください");
            }
            else
            {
                if (g_trigOn)
                    DoTrigger();
                bool ok = Process::Write32(SlotAddr(), g_destId);
                if (!g_wasDown)
                    OSD::Notify(std::string(ok ? "書き込みOK  " : "書き込み失敗  ") + DescribeAddr(SlotAddr()));
            }
        }
        g_wasDown = down;
    }

    // ----- マップIDの枠を探す(メモリ全体から、一覧にあるマップIDと同じ値を探す) -----
    struct Hit
    {
        u32 addr;
        u32 id;
    };
    static std::vector<Hit> g_hits, g_prevHits;
    static u32 g_sortedIds[400];
    static u32 g_sortedCount = 0;
    static u8  g_lowBits[8192];
    static bool g_idsReady = false;

    static void PrepareIds(void)
    {
        if (g_idsReady)
            return;
        for (u32 i = 0; i < g_mapCount; i++)
            g_sortedIds[i] = g_maps[i].id;
        std::sort(g_sortedIds, g_sortedIds + g_mapCount);
        g_sortedCount = (u32)(std::unique(g_sortedIds, g_sortedIds + g_mapCount) - g_sortedIds);
        memset(g_lowBits, 0, sizeof(g_lowBits));
        for (u32 i = 0; i < g_sortedCount; i++)
        {
            u32 lo = g_sortedIds[i] & 0xFFFF;
            g_lowBits[lo >> 3] |= (u8)(1u << (lo & 7));
        }
        g_idsReady = true;
    }

    static void ScanMapSlots(void)
    {
        PrepareIds();
        g_prevHits = g_hits;
        g_hits.clear();

        u32 addr = 0x08000000;
        while (addr < 0x10000000 && g_hits.size() < 300)
        {
            MemInfo mi;
            PageInfo pi;
            if (svcQueryMemory(&mi, &pi, addr) < 0)
                break;
            u32 end = mi.base_addr + mi.size;
            if (end <= addr)
                break;
            if ((mi.perm & MEMPERM_READ) && mi.state != MEMSTATE_FREE && mi.state != MEMSTATE_IO)
            {
                u32 a = (mi.base_addr > addr) ? mi.base_addr : addr;
                u32 stop = (end < 0x10000000) ? end : 0x10000000;
                for (; a + 4 <= stop; a += 4)
                {
                    u32 v = *(volatile u32 *)a;
                    u32 lo = v & 0xFFFF;
                    if (!(g_lowBits[lo >> 3] & (1u << (lo & 7))))
                        continue;
                    if (std::binary_search(g_sortedIds, g_sortedIds + g_sortedCount, v))
                    {
                        Hit h = {a, v};
                        g_hits.push_back(h);
                        if (g_hits.size() >= 300)
                            break;
                    }
                }
            }
            addr = end;
        }
    }

    static void FindSlotMenu(void)
    {
        for (;;)
        {
            std::vector<std::string> opts =
            {
                "メモリをスキャン",
                "結果から枠を選ぶ",
                "今の枠: " + Hex8(SlotAddr()),
                "枠のアドレスを入力(16進)",
            };
            Keyboard kb("マップ枠を探す", opts);
            int r = kb.Open();
            if (r < 0)
                return;

            if (r == 0)
            {
                OSD::Notify("メモリをスキャン中... しばらくお待ちください");
                ScanMapSlots();
                ShowInfo("スキャン完了: " + Dec((u32)g_hits.size()) + "件\nマップを移動してもう一度スキャンすると、\n値が変わったアドレスが枠です。");
            }
            else if (r == 1)
            {
                // 前回と値が変わった場所を先頭に並べる
                std::vector<Hit> list;
                std::vector<std::string> labels;
                for (size_t i = 0; i < g_hits.size(); i++)
                {
                    bool changed = false;
                    for (size_t j = 0; j < g_prevHits.size(); j++)
                        if (g_prevHits[j].addr == g_hits[i].addr && g_prevHits[j].id != g_hits[i].id)
                            changed = true;
                    if (changed)
                    {
                        list.push_back(g_hits[i]);
                        const char *n = MapNameOf(g_hits[i].id);
                        labels.push_back("[変化] " + Hex8(g_hits[i].addr) + " " + (n ? n : "?"));
                    }
                }
                for (size_t i = 0; i < g_hits.size() && list.size() < 200; i++)
                {
                    bool already = false;
                    for (size_t j = 0; j < list.size(); j++)
                        if (list[j].addr == g_hits[i].addr)
                            already = true;
                    if (already)
                        continue;
                    list.push_back(g_hits[i]);
                    const char *n = MapNameOf(g_hits[i].id);
                    labels.push_back(Hex8(g_hits[i].addr) + " " + (n ? n : "?"));
                }
                if (list.empty())
                {
                    ShowInfo("結果がありません。先にスキャンしてください。");
                    continue;
                }
                Keyboard pk("今のマップが入っている枠", labels);
                int pr = pk.Open();
                if (pr >= 0 && pr < (int)list.size())
                {
                    g_slot = list[pr].addr;
                    OSD::Notify("枠を設定: " + Hex8(g_slot));
                }
            }
            else if (r == 3)
            {
                u32 v = SlotAddr();
                Keyboard hex("枠のアドレス(0=既定)");
                hex.IsHexadecimal(true);
                if (hex.Open(v, SlotAddr()) >= 0)
                    g_slot = v;
            }
        }
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
            "フィールド(g)", "屋内(i)", "ダンジョン等(d)", "戦闘(b)",
            "駅(s)", "イベント(e)", "その他", "16進で入力"
        };
        Keyboard kb("カテゴリ", cats);
        int c = kb.Open();
        if (c < 0)
            return;

        if (c == 7)
        {
            u32 v = g_destId;
            Keyboard hex("マップID(16進)");
            hex.IsHexadecimal(true);
            if (hex.Open(v, g_destId) >= 0)
            {
                g_destId = v;
                g_destName = "(hex input)";
                OSD::Notify("マップIDを設定しました");
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

        Keyboard list("マップを選択", names);
        int s = list.Open();
        if (s < 0 || s >= (int)idx.size())
            return;

        g_destId = g_maps[idx[s]].id;
        g_destName = g_maps[idx[s]].name;
        OSD::Notify("行き先: " + g_destName);
    }

    static void MapWarpMenu(MenuEntry *entry)
    {
        static const char *hotkeyNames[] =
        {
            "L+R", "L+Up", "L+Down", "L+Left", "L+Right", "A+B", "X+Y"
        };
        for (;;)
        {
            std::vector<std::string> opts =
            {
                "行き先: " + TruncU8(g_destName, 16),
                "マップ枠を探す(今: " + Hex8(SlotAddr()) + ")",
                std::string("ホットキー: ") + hotkeyNames[g_hotkey],
                std::string("きっかけ(Y座標): ") + (g_trigOn ? "ON" : "OFF"),
                "きっかけのアドレス: " + Hex8(TrigAddr()),
                "現在の値を確認",
                std::string("配置: ") + (g_layoutMode == 0 ? "自動" : (g_layoutMode == 1 ? "New3DS" : (g_layoutMode == 2 ? "Old3DS" : "Citra")))
                    + "(" + LayoutName() + ")",
            };
            Keyboard kb("マップワープ(ホットキー長押し)", opts);
            int r = kb.Open();
            if (r < 0)
                return;

            if (r == 0)
                SelectDestination();
            else if (r == 1)
                FindSlotMenu();
            else if (r == 2)
            {
                std::vector<std::string> hk(hotkeyNames, hotkeyNames + 7);
                Keyboard hk2("ホットキー", hk);
                int hr = hk2.Open();
                if (hr >= 0) g_hotkey = hr;
            }
            else if (r == 3)
                g_trigOn = !g_trigOn;
            else if (r == 4)
            {
                u32 v = TrigAddr();
                Keyboard hex("きっかけのアドレス(0=既定)");
                hex.IsHexadecimal(true);
                if (hex.Open(v, TrigAddr()) >= 0)
                    g_trigAddr = v;
            }
            else if (r == 6)
            {
                std::vector<std::string> lm = {"自動判定", "New3DS", "Old3DS", "Citra/Azahar"};
                Keyboard lk("メモリ配置", lm);
                int lr = lk.Open();
                if (lr >= 0) g_layoutMode = lr;
            }
            else if (r == 5)
            {
                ShowInfo(std::string("配置: ") + LayoutName() + "\n"
                         "枠 " + DescribeAddr(SlotAddr()) + "\n"
                         "きっかけ " + DescribeAddr(TrigAddr()) + "\n"
                         "X " + DescribeAddr(PLAYER_X_ADDR) + "\n"
                         "Y " + DescribeAddr(PLAYER_Y_ADDR) + "\n"
                         "Z " + DescribeAddr(PLAYER_Z_ADDR));
            }
        }
    }

    // ================= アイテム作成 =================
    // wikiのARコード「アイテム作成」(ARM機械語)を、そのままの呼び出しに直したもの:
    //   0x005DD738 の関数を (r0 = 0x0080D6BE, r1 = アイテムID, r2 = 個数) で呼ぶ。
    //   元のコードは Start を押した瞬間(押した状態に変わった時)に1回だけ呼んでいた。
    // コード領域のアドレスなので、Old/New/Citra で共通。
    // アイテムID一覧(IDは yo-docs の YW2 Item IDs。名前はゲームの item_text_ja / chara_text_ja から取り出した正式な日本語名): 種類 0=消耗品 1=大事なもの 2=装備品 3=ソウル 4=妖怪
    struct ItemInfo
    {
        const char *name;
        u32         id;
        int         cat;
    };
    static const ItemInfo g_items[] =
    {
        {"チョコボー", 0x8162B121, 0},
        {"うめおにぎり", 0x6B85C07A, 0},
        {"たかなおにぎり", 0xF28C91C0, 0},
        {"いくらおにぎり", 0x858BA156, 0},
        {"あいじょう天むす", 0x1BEF34F5, 0},
        {"サンドイッチ", 0x6A47AA4D, 0},
        {"クリームパン", 0xF34EFBF7, 0},
        {"カレーパン", 0x6D2A6E54, 0},
        {"フランスパン", 0x8449CB61, 0},
        {"アッカンベーグル", 0x1A2D5EC2, 0},
        {"１０円ガム", 0x68011414, 0},
        {"ねりあめ", 0xF10845AE, 0},
        {"大きなえびせんべい", 0x860F7538, 0},
        {"ドロップ缶", 0x186BE09B, 0},
        {"かき氷", 0x6F6CD00D, 0},
        {"りんごあめ", 0xF66581B7, 0},
        {"牛乳", 0x69C37E23, 0},
        {"コーヒー牛乳", 0xF0CA2F99, 0},
        {"フルーツ牛乳", 0x87CD1F0F, 0},
        {"ぜっぴん牛乳", 0x19A98AAC, 0},
        {"ヨカコーラ", 0x6C8C68A6, 0},
        {"妖緑茶", 0xF585391C, 0},
        {"栄妖ドリンク", 0x8282098A, 0},
        {"ヨキシマムゴッド", 0x1CE69C29, 0},
        {"ハンバーガー", 0x6D4E0291, 0},
        {"チーズバーガー", 0xF447532B, 0},
        {"ダブルバーガー", 0x834063BD, 0},
        {"モグモグバーガー", 0x1D24F61E, 0},
        {"カップラーメン", 0x6ECAD6FF, 0},
        {"とんこつラーメン", 0xF7C38745, 0},
        {"チャーシューメン", 0x80C4B7D3, 0},
        {"スペシャルラーメン", 0x1EA02270, 0},
        {"かっぱ巻き", 0x659691C2, 0},
        {"エビ", 0xFC9FC078, 0},
        {"いくら巻き", 0x8B98F0EE, 0},
        {"大トロ", 0x15FC654D, 0},
        {"ぎょうざ", 0x6454FBF5, 0},
        {"レバニラ", 0xFD5DAA4F, 0},
        {"カニ玉", 0x8A5A9AD9, 0},
        {"エビチリ", 0x143E0F7A, 0},
        {"ヘルシーマーボー", 0x63393FEC, 0},
        {"にんじん", 0xD2FBCD28, 0},
        {"きゅうり", 0x4BF29C92, 0},
        {"たけのこ", 0x3CF5AC04, 0},
        {"まつたけ", 0xA29139A7, 0},
        {"とりもも", 0xD0BD7371, 0},
        {"豚バラブロック", 0x49B422CB, 0},
        {"牛タン", 0x3EB3125D, 0},
        {"特上しもふり", 0xA0D787FE, 0},
        {"アジのひもの", 0xD17F1946, 0},
        {"ブリの切り身", 0x487648FC, 0},
        {"しんせんなウニ", 0x3F71786A, 0},
        {"極上マグロ", 0xA115EDC9, 0},
        {"チキンカレー", 0xD4300FC3, 0},
        {"マトンカレー", 0x4D395E79, 0},
        {"シーフードカレー", 0x3A3E6EEF, 0},
        {"ナマステカレー", 0xA45AFB4C, 0},
        {"野菜ごろごろカレー", 0xD35DCBDA, 0},
        {"チーズケーキ", 0xD5F265F4, 0},
        {"ショートケーキ", 0x4CFB344E, 0},
        {"ロイヤルパンケーキ", 0x3BFC04D8, 0},
        {"特大ジャンボパフェ", 0xA598917B, 0},
        {"わらびもち", 0xD29FA1ED, 0},
        {"元祖まんじゅう", 0x4B96F057, 0},
        {"本家まんじゅう", 0x3C91C0C1, 0},
        {"ほっこりだいこん", 0xD7B4DBAD, 0},
        {"味しみたまご", 0x4EBD8A17, 0},
        {"やわらか牛スジ", 0x39BABA81, 0},
        {"極上のおでん", 0xA7DE2F22, 0},
        {"ざるそば", 0xD676B19A, 0},
        {"じゃがじゃがチップス", 0xDD2AF6A7, 0},
        {"バリうまスナック", 0x4423A71D, 0},
        {"どっさりチーズコーン", 0x3324978B, 0},
        {"雪かきピー", 0xAD400228, 0},
        {"ミニけいけんちだま", 0x8D447F63, 0},
        {"小けいけんちだま", 0x1320EAC0, 0},
        {"中けいけんちだま", 0x8A29BB7A, 0},
        {"大けいけんちだま", 0xFD2E8BEC, 0},
        {"超けいけんちだま", 0x634A1E4F, 0},
        {"神けいけんちだま", 0x144D2ED9, 0},
        {"スタミナム", 0x17AD9672, 0},
        {"スタミナムアルファ", 0x8EA4C7C8, 0},
        {"攻めの秘伝書", 0x10A43EAE, 0},
        {"術の秘伝書", 0x89AD6F14, 0},
        {"ひっさつの秘伝書", 0xFEAA5F82, 0},
        {"まじめに生きる", 0x1BF87993, 0},
        {"習得！カラテ講座", 0x82F12829, 0},
        {"応用！カラテ講座", 0xF5F618BF, 0},
        {"ようじゅつ大百科", 0x6B928D1C, 0},
        {"ようじゅつ超百科", 0x1C95BD8A, 0},
        {"ガードのすすめ", 0x859CEC30, 0},
        {"ガードばんざい", 0xF29BDCA6, 0},
        {"いやしの天使ちゃん", 0x6224C137, 0},
        {"さよなら天使ちゃん", 0x1523F1A1, 0},
        {"おじゃま道", 0x75E47844, 0},
        {"続おじゃま道", 0x02E348D2, 0},
        {"サポートライフ７月号", 0x9BEA1968, 0},
        {"サポートライフ特別編", 0xECED29FE, 0},
        {"ちからのおふだ", 0x15EB282B, 0},
        {"ようりょくのおふだ", 0x8CE27991, 0},
        {"まもりのおふだ", 0xFBE54907, 0},
        {"すばやさのおふだ", 0x6581DCA4, 0},
        {"まず〜い漢方", 0x1429421C, 0},
        {"にが〜い漢方", 0x8D2013A6, 0},
        {"ふか〜い漢方", 0xFA272330, 0},
        {"みがわり人形", 0x166FFC45, 0},
        {"鉄のこけし", 0x610CA016, 0},
        {"銅のこけし", 0x11665499, 0},
        {"銀のこけし", 0x886F0523, 0},
        {"金のこけし", 0xFF6835B5, 0},
        {"プラチナのこけし", 0x160B9080, 0},
        {"魚良のまきえ", 0xAD574F4E, 0},
        {"スーパー黒みつ", 0x345E1EF4, 0},
        {"ダンシングスター", 0xAC952579, 0},
        {"クジ引き券", 0x359C74C3, 0},
        {"ミュージックカード", 0x429B4455, 0},
        {"銅の手形", 0x5B807514, 0},
        {"銀の手形", 0xC5E4E0B7, 0},
        {"金の手形", 0xB2E3D021, 0},
        {"怪魔の素", 0x2C874582, 0},
        {"名刀マサムネ", 0x1A3A13A4, 0},
        {"妖刀ムラマサ", 0x8333421E, 0},
        {"神剣クサナギ", 0x845E8607, 0},
        {"大将の魂", 0xF4347288, 0},
        {"モテウェポン", 0x6A50E72B, 0},
        {"ギガヘルツ玉", 0x1D57D7BD, 0},
        {"無双の魂", 0x63E6AB00, 0},
        {"プラチナインゴット", 0x9A28735F, 0},
        {"雪王のマント", 0x14E19B96, 0},
        {"愛のシャク", 0x74261273, 0},
        {"白銀のかみどめ", 0xF359B691, 0},
        {"親方ダンベル", 0x032122E5, 0},
        {"邪心のかたまり", 0x734BD66A, 0},
        {"生命のおしろい", 0xED2F43C9, 0},
        {"龍神玉", 0x9D45B746, 0},
        {"暴虐な刀", 0x417DB4E8, 0},
        {"砂スーツ", 0x367A847E, 0},
        {"神通力水", 0xAF73D5C4, 0},
        {"呪いの日記帳", 0xD874E552, 0},
        {"爆音クラクション", 0x48CBF8C3, 0},
        {"記憶吸い取り機", 0x3FCCC855, 0},
        {"人魚の宝玉", 0x0951E636, 0},
        {"愛情おむすび", 0x69966FD3, 0},
        {"ボロボロの鈴", 0x7E56D6A0, 0},
        {"サビれた刀", 0x793B12B9, 0},
        {"荒々しい砥石", 0xE0324303, 0},
        {"禍々しい砥石", 0x97357395, 0},
        {"神々しい砥石", 0x078A6E04, 0},
        {"木彫りのクマ", 0x104AD777, 0},
        {"金魚のちょうちん", 0x674DE7E1, 0},
        {"匠のちょうちん", 0xFE44B65B, 0},
        {"金のしゃちほこ", 0x894386CD, 0},
        {"たこ焼き器", 0x1727136E, 0},
        {"砂丘の砂", 0x602023F8, 0},
        {"伝統のあみ傘", 0xF9297242, 0},
        {"ハニワ", 0x8E2E42D4, 0},
        {"真っ赤なハイビスカス", 0x1E915F45, 0},
        {"いやしの草", 0xE75F871A, 0},
        {"くさみの草", 0x9058B78C, 0},
        {"にがみの草", 0x0E3C222F, 0},
        {"赤コイン", 0x9A89BF7C, 0},
        {"黄色コイン", 0x0380EEC6, 0},
        {"オレンジコイン", 0x7487DE50, 0},
        {"桃コイン", 0xEAE34BF3, 0},
        {"緑コイン", 0x9DE47B65, 0},
        {"青コイン", 0x04ED2ADF, 0},
        {"紫コイン", 0x73EA1A49, 0},
        {"水色コイン", 0xE35507D8, 0},
        {"５つ星コイン", 0xCB567D2C, 0},
        {"スペシャルコイン", 0x5BE960BD, 0},
        {"どきどきコイン（妖）", 0x2CEE502B, 0},
        {"どきどきコイン（怪）", 0x4C29D9CE, 0},
        {"どきどきコイン（召）", 0x3B2EE958, 0},
        {"どきどきコイン（喚）", 0xA227B8E2, 0},
        {"わくわくコイン（花）", 0x35F5616A, 0},
        {"わくわくコイン（鳥）", 0x67048A0D, 0},
        {"わくわくコイン（風）", 0x1003BA9B, 0},
        {"わくわくコイン（月）", 0x890AEB21, 0},
        {"きらきらコイン", 0xFE0DDBB7, 0},
        {"ご当地コイン（北）", 0x60694E14, 0},
        {"ご当地コイン（東北）", 0x176E7E82, 0},
        {"ご当地コイン（関東）", 0x8E672F38, 0},
        {"ご当地コイン（中部）", 0xF9601FAE, 0},
        {"ご当地コイン（近畿）", 0x69DF023F, 0},
        {"ご当地コイン（中国）", 0x1ED832A9, 0},
        {"ご当地コイン（四国）", 0x7E1FBB4C, 0},
        {"ご当地コイン（九州）", 0x09188BDA, 0},
        {"ご当地コイン（沖縄）", 0x9011DA60, 0},
        {"あやかしコイン", 0xE716EAF6, 0},
        {"あやかしコイン（猪）", 0x79727F55, 0},
        {"あやかしコイン（鹿）", 0x0E754FC3, 0},
        {"あやかしコイン（蝶）", 0x977C1E79, 0},
        {"うきうきコイン", 0xE07B2EEF, 0},
        {"赤コインの欠片", 0xDCFFD1F6, 0},
        {"黄色コインの欠片", 0xABF8E160, 0},
        {"オレンジコインの欠片", 0x32F1B0DA, 0},
        {"桃コインの欠片", 0x45F6804C, 0},
        {"緑コインの欠片", 0xD5499DDD, 0},
        {"青コインの欠片", 0xA24EAD4B, 0},
        {"紫コインの欠片", 0xC28924AE, 0},
        {"水色コインの欠片", 0xB58E1438, 0},
        {"赤い箱", 0x47874761, 0},
        {"ゲートボール", 0x46452D56, 0},
        {"ツチノコの玉", 0x883C1092, 0},
        {"キュン玉", 0x11354128, 0},
        {"妖怪ウォッチ①", 0x4F6B8097, 1},
        {"妖怪ウォッチ②", 0xD662D12D, 1},
        {"妖怪ウォッチ・零式", 0x1D9A6BF0, 1},
        {"ふしぎな時計①", 0x36B73833, 1},
        {"ふしぎな時計②", 0x41B008A5, 1},
        {"妖怪大辞典", 0xA165E1BB, 1},
        {"虫あみ", 0x4806448E, 1},
        {"釣りざお", 0xD10F1534, 1},
        {"博物館のチケット", 0xA60825A2, 1},
        {"そよかぜの自転車", 0x5670B1D6, 1},
        {"はるかぜの自転車", 0xCF79E06C, 1},
        {"夕焼けの自転車①", 0x8755B2B2, 1},
        {"海原の自転車①", 0xF0528224, 1},
        {"お花見の自転車①", 0x60ED9FB5, 1},
        {"若葉の自転車①", 0x17EAAF23, 1},
        {"わた雲の自転車①", 0x772D26C6, 1},
        {"おひさまの自転車①", 0x002A1650, 1},
        {"みわくの自転車①", 0x992347EA, 1},
        {"くらやみの自転車①", 0xEE24777C, 1},
        {"黄金の自転車①", 0x7040E2DF, 1},
        {"夕焼けの自転車②", 0x0747D249, 1},
        {"海原の自転車②", 0x9E4E83F3, 1},
        {"お花見の自転車②", 0xE949B365, 1},
        {"若葉の自転車②", 0x79F6AEF4, 1},
        {"わた雲の自転車②", 0x0EF19E62, 1},
        {"おひさまの自転車②", 0xF0B53A09, 1},
        {"みわくの自転車②", 0x87B20A9F, 1},
        {"くらやみの自転車②", 0x1EBB5B25, 1},
        {"黄金の自転車②", 0x69BC6BB3, 1},
        {"エラベールコイン", 0x93538339, 1},
        {"超エラベールコイン", 0x21778140, 1},
        {"１日乗車券", 0x943E4720, 1},
        {"らくらくフリーパス", 0x7A30260C, 1},
        {"スタンプラリーの台紙", 0x1341E3C2, 1},
        {"宿題ドリル", 0xB87ED0FA, 1},
        {"すご〜い漢方", 0x261A4559, 1},
        {"匠のネジ回し", 0x511D75CF, 1},
        {"元祖まんじゅう", 0xC8142475, 1},
        {"本家まんじゅう", 0xBF1314E3, 1},
        {"おばあちゃん家メモ", 0x2FAC0972, 1},
        {"校内のカギ", 0x58AB39E4, 1},
        {"コマさんの手紙", 0x8A48B278, 1},
        {"もんげ〜鈴", 0x142C27DB, 1},
        {"ヤバネロ", 0x632B174D, 1},
        {"セイコーン時計", 0xFA2246F7, 1},
        {"ふしぎなビー玉", 0x8D257661, 1},
        {"セキカトケール", 0x6A9D5B66, 1},
        {"牛乳ビンのフタ", 0x5C007505, 1},
        {"ガッツ仮面メンコ", 0x2B074593, 1},
        {"小屋のカギ", 0xE454B3AF, 1},
        {"メガウォッチ", 0x0D37169A, 1},
        {"ホース", 0x3F017418, 1},
        {"屋敷のカギ 裏口", 0x0A5AD283, 1},
        {"屋敷のカギ 大", 0x6446D354, 1},
        {"屋敷のカギ 小", 0xFD4F82EE, 1},
        {"妖魔界へのカギ", 0xE33977B6, 1},
        {"廃屋のカギ", 0x04815AB1, 1},
        {"あやかしのカギ（猪）", 0x77CA9EEB, 1},
        {"あやかしのカギ（鹿）", 0xE775837A, 1},
        {"あやかしのカギ（蝶）", 0x73866A27, 1},
        {"親方の油", 0x7D5DE215, 1},
        {"ニャーＫＢの生写真 @", 0xC264E0A6, 1},
        {"ニャーＫＢの生写真 A", 0x52DBFD37, 1},
        {"ニャーＫＢの生写真 B", 0x25DCCDA1, 1},
        {"ニャーＫＢの生写真 C", 0x451B4444, 1},
        {"ニャーＫＢの生写真 D", 0x321C74D2, 1},
        {"ニャーＫＢの生写真 E", 0xAB152568, 1},
        {"ニャーＫＢの生写真 F", 0xDC1215FE, 1},
        {"ニャーＫＢの生写真 G", 0x4276805D, 1},
        {"ニャーＫＢの生写真 H", 0x3571B0CB, 1},
        {"ニャーＫＢの生写真 I", 0xAC78E171, 1},
        {"ケツプリドラゴン", 0xDB7FD1E7, 1},
        {"Ｃ‐３０２号室のカギ", 0x4BC0CC76, 1},
        {"Ａ‐２０１号室のカギ", 0x3CC7FCE0, 1},
        {"Ａ‐２０３号室のカギ", 0x6E361787, 1},
        {"Ｂ‐１０２号室のカギ", 0x19312711, 1},
        {"Ｂ‐２０４号室のカギ", 0x803876AB, 1},
        {"Ｂ‐３０１号室のカギ", 0xF73F463D, 1},
        {"Ｃ‐１０１号室のカギ", 0x695BD39E, 1},
        {"Ｃ‐３０３号室のカギ", 0x1E5CE308, 1},
        {"怪獣映画のポスター", 0xF7D8FE10, 1},
        {"ロックスターの衣装", 0x80DFCE86, 1},
        {"ヒーローの看板", 0x19D69F3C, 1},
        {"白黒アニメのフィルム", 0x6ED1AFAA, 1},
        {"カレーの看板", 0xFE6EB23B, 1},
        {"大女優の衣装", 0x896982AD, 1},
        {"ラッコちゃん", 0xE9AE0B48, 1},
        {"ヨカコーラの瓶", 0x9EA93BDE, 1},
        {"アヒルのおまる", 0x07A06A64, 1},
        {"カメカメラの人形", 0x70A75AF2, 1},
        {"ニャイーンの巻物", 0xEEC3CF51, 1},
        {"コマニャチの巻物", 0x99C4FFC7, 1},
        {"ニョエーの巻物", 0x00CDAE7D, 1},
        {"天地まんじゅう", 0xF7D7E7F2, 1},
        {"大事な忘れもの", 0x6EDEB648, 1},
        {"どんどろの素", 0x87BD137D, 1},
        {"赤いカードキー", 0xF0BA23EB, 1},
        {"青いカードキー", 0x69B37251, 1},
        {"ゴールドカードキー", 0x1EB442C7, 1},
        {"ゴツいベルト", 0x8E0B5F56, 1},
        {"なまくらガタナ", 0xF90C6FC0, 1},
        {"レトロでキレイなおもちゃ", 0x99CBE625, 1},
        {"レトロでふるびたおもちゃ", 0xEECCD6B3, 1},
        {"お宝の地図", 0x77C58709, 1},
        {"びしょびしょの地図", 0x00C2B79F, 1},
        {"しわくちゃの地図", 0x9EA6223C, 1},
        {"シミつきの地図", 0xE9A112AA, 1},
        {"新たなるお宝の地図", 0x70A84310, 1},
        {"新たなるお宝のカギ", 0x07AF7386, 1},
        {"最後のお宝の地図", 0x97106E17, 1},
        {"なぞのハンドル", 0xE0175E81, 1},
        {"白黒カメラ", 0xB2E6B5E6, 1},
        {"昔の小学校の写真", 0xC5E18570, 1},
        {"花道商店街の写真", 0x5CE8D4CA, 1},
        {"ヒョッシーの写真", 0x2BEFE45C, 1},
        {"ニャーＫＢのサイン", 0xB58B71FF, 1},
        {"おてせい手ぬぐい", 0xC28C4169, 1},
        {"ウバウネのクシ", 0xE562481C, 1},
        {"ダンスダンス革命", 0x7C6B19A6, 1},
        {"妖魔特急パス", 0x0B6C2930, 1},
        {"ゲラゲランドの入場券", 0x9508BC93, 1},
        {"ごくらく温泉の入場券", 0xE20F8C05, 1},
        {"きもだ飯の入場券", 0x7B06DDBF, 1},
        {"天ノ湯のお札", 0x0C01ED29, 1},
        {"山ノ湯のお札", 0x9CBEF0B8, 1},
        {"丘ノ湯のお札", 0xEBB9C02E, 1},
        {"道ノ湯のお札", 0x8B7E49CB, 1},
        {"森ノ湯のお札", 0xFC79795D, 1},
        {"福ノ湯のお札", 0x657028E7, 1},
        {"富ノ湯のお札", 0x12771871, 1},
        {"ダークベルト", 0x8C138DD2, 1},
        {"ダークマント", 0xFB14BD44, 1},
        {"ブレーキハンドル", 0x621DECFE, 1},
        {"ふくふく超特急パス", 0x151ADC68, 1},
        {"福ガシャコイン", 0x85A5C1F9, 1},
        {"ガッツの紋章", 0xF2A2F16F, 1},
        {"だいじなアイテム２０", 0xA0531A08, 1},
        {"だいじなアイテム２１", 0xD7542A9E, 1},
        {"だいじなアイテム２２", 0x4E5D7B24, 1},
        {"だいじなアイテム２３", 0x395A4BB2, 1},
        {"だいじなアイテム２４", 0xA73EDE11, 1},
        {"だいじなアイテム２５", 0xD039EE87, 1},
        {"メロンの種", 0xD0CD7F03, 1},
        {"ミカンの種", 0xA7CA4F95, 1},
        {"キウイの種", 0x37755204, 1},
        {"ブドウの種", 0x40726292, 1},
        {"イチゴの種", 0x20B5EB77, 1},
        {"スイカの種", 0x57B2DBE1, 1},
        {"ノガッパの歯車", 0xCEBB8A5B, 1},
        {"コマさんの歯車", 0xB9BCBACD, 1},
        {"ひも爺の歯車", 0x27D82F6E, 1},
        {"じんめん犬の歯車", 0x50DF1FF8, 1},
        {"ツチノコの歯車", 0xC9D64E42, 1},
        {"りゅーくんの歯車", 0xBED17ED4, 1},
        {"Ｆ型の鈴", 0x2E6E6345, 1},
        {"ハートの鈴", 0x596953D3, 1},
        {"トラの鈴", 0x0B98B8B4, 1},
        {"王国の鈴", 0x7C9F8822, 1},
        {"ジェットな鈴", 0xE596D998, 1},
        {"地縛霊のふろしき", 0x9291E90E, 1},
        {"思い出のフタ", 0x0CF57CAD, 1},
        {"ダークな鈴", 0x7BF24C3B, 1},
        {"そまつなうでわ", 0x4511AA26, 2},
        {"安っぽいうでわ", 0xDB753F85, 2},
        {"ロックなうでわ", 0xDC18FB9C, 2},
        {"パワフルなうでわ", 0xAC720F13, 2},
        {"超力のうでわ", 0xAB1FCB0A, 2},
        {"豪気なうでわ", 0x3CCD1282, 2},
        {"太陽のうでわ", 0x357B5EA9, 2},
        {"彗星のうでわ", 0x4BCA2214, 2},
        {"鬼神のうでわ", 0x427C6E3F, 2},
        {"伝説のうでわ", 0x2B0DABF1, 2},
        {"さびたゆびわ", 0x44D3C011, 2},
        {"ダサいゆびわ", 0xDAB755B2, 2},
        {"プリティーリング", 0xDDDA91AB, 2},
        {"レインボーリング", 0xADB06524, 2},
        {"げんえいのゆびわ", 0xAADDA13D, 2},
        {"妖精のゆびわ", 0x3D0F78B5, 2},
        {"月光のゆびわ", 0x34B9349E, 2},
        {"運命のゆびわ", 0x4A084823, 2},
        {"鬼神のゆびわ", 0x43BE0408, 2},
        {"伝説のゆびわ", 0x2ACFC1C6, 2},
        {"古びたおまもり", 0x46957E48, 2},
        {"ボロボロなおまもり", 0xD8F1EBEB, 2},
        {"ルーンのおまもり", 0xDF9C2FF2, 2},
        {"加護のおまもり", 0xAFF6DB7D, 2},
        {"てっぺきのおまもり", 0xA89B1F64, 2},
        {"幸運のおまもり", 0x3F49C6EC, 2},
        {"星雲のおまもり", 0x36FF8AC7, 2},
        {"大地のおまもり", 0x484EF67A, 2},
        {"鬼神のおまもり", 0x41F8BA51, 2},
        {"伝説のおまもり", 0x28897F9F, 2},
        {"シンプルバッジ", 0x4757147F, 2},
        {"ブラックバッジ", 0xD93381DC, 2},
        {"ぴかぴかバッジ", 0xDE5E45C5, 2},
        {"かわいいバッジ", 0xAE34B14A, 2},
        {"はやてのバッジ", 0xA9597553, 2},
        {"オーロラバッジ", 0x3E8BACDB, 2},
        {"流星のバッジ", 0x373DE0F0, 2},
        {"雷鳴のバッジ", 0x498C9C4D, 2},
        {"鬼神のバッジ", 0x403AD066, 2},
        {"伝説のバッジ", 0x294B15A8, 2},
        {"セミ忍刀", 0x421802FA, 2},
        {"マッスルベル", 0xDB115340, 2},
        {"マジカルベル", 0xAC1663D6, 2},
        {"タフベル", 0x3272F675, 2},
        {"スピードベル", 0x4575C6E3, 2},
        {"ムゲンすいとう", 0xDC7C9759, 2},
        {"天狗のうちわ", 0xAB7BA7CF, 2},
        {"ハッピーはっぴ", 0x3BC4BA5E, 2},
        {"釘バット", 0x4CC38AC8, 2},
        {"どんちゃんバチ", 0x5C6EF7A2, 2},
        {"ロボビタミンＥ", 0xC567A618, 2},
        {"ブリーバンド", 0xB260968E, 2},
        {"思い出の鈴", 0x22DF8B1F, 2},
        {"元祖はちまき", 0xB50D5297, 2},
        {"本家はちまき", 0x2B69C734, 2},
        {"逆転のつるぎ", 0x2C04032D, 2},
        {"逆転のまがたま", 0x5B0333BB, 2},
        {"逆転のかがみ", 0xC20A6201, 2},
        {"極楽玉", 0x55D8BB89, 2},
        {"破邪のお札", 0x43DA68CD, 2},
        {"呪言の刀", 0xDAD33977, 2},
        {"呪言の杖", 0xADD409E1, 2},
        {"呪言の盾", 0x33B09C42, 2},
        {"呪言の袴", 0x44B7ACD4, 2},
        {"てかげんベルト", 0x419CD694, 2},
        {"おさるの輪っか", 0xAF92B7B8, 2},
        {"元帥の勲章", 0xFDADCD43, 2},
        {"摂政の勲章", 0x64A49CF9, 2},
        {"関白の勲章", 0x13A3AC6F, 2},
        {"右大臣の勲章", 0x8DC739CC, 2},
        {"左大臣の勲章", 0xFAC0095A, 2},
        {"大将の勲章", 0x63C958E0, 2},
        {"中将の勲章", 0x14CE6876, 2},
        {"少将の勲章", 0x847175E7, 2},
        {"ぶようじん坊", 0x10F16846, 3},
        {"がらあきん坊", 0x67F658D0, 3},
        {"さきがけの助", 0xFEFF096A, 3},
        {"つづかな僧", 0x89F839FC, 3},
        {"やめたい師", 0x179CAC5F, 3},
        {"もうせん和尚", 0x609B9CC9, 3},
        {"ちからモチ", 0xF992CD73, 3},
        {"やきモチ", 0x8E95FDE5, 3},
        {"ダラケ刀", 0x1E2AE074, 3},
        {"ザンバラ刀", 0x692DD0E2, 3},
        {"ダララだんびら", 0x09EA5907, 3},
        {"しょうブシ", 0x7EED6991, 3},
        {"まさむね", 0xE7E4382B, 3},
        {"くさなぎ", 0x90E308BD, 3},
        {"むらまさ", 0x0E879D1E, 3},
        {"カブトさん", 0x7980AD88, 3},
        {"ゲンマ将軍", 0xE089FC32, 3},
        {"黄泉ゲンスイ", 0x978ECCA4, 3},
        {"メラメライオン", 0x0731D135, 3},
        {"グラグライオン", 0x7036E1A3, 3},
        {"とどろき獅子", 0x22C70AC4, 3},
        {"クワノ武士", 0x55C03A52, 3},
        {"クワガ大将①", 0xCCC96BE8, 3},
        {"クワガ大将②", 0xBBCE5B7E, 3},
        {"オオクワノ神", 0x25AACEDD, 3},
        {"ベンケイ", 0x52ADFE4B, 3},
        {"からくりベンケイ", 0xCBA4AFF1, 3},
        {"くしゃ武者", 0xBCA39F67, 3},
        {"なまはげ", 0x2C1C82F6, 3},
        {"ズルズルづる", 0x5B1BB260, 3},
        {"かぜカモ", 0x3BDC3B85, 3},
        {"さとりちゃん", 0x4CDB0B13, 3},
        {"よつめ", 0xD5D25AA9, 3},
        {"わすれん帽", 0xA2D56A3F, 3},
        {"ばか頭巾", 0x3CB1FF9C, 3},
        {"わらえ姉", 0x4BB6CF0A, 3},
        {"青くちびる", 0xD2BF9EB0, 3},
        {"やまと", 0xA5B8AE26, 3},
        {"カブキ猿", 0x3507B3B7, 3},
        {"モテモ天", 0x42008321, 3},
        {"モテマクール", 0x749DAD42, 3},
        {"モテヌス", 0x039A9DD4, 3},
        {"バクロ婆", 0x9A93CC6E, 3},
        {"ドクロ婆", 0xED94FCF8, 3},
        {"ババァーン", 0x73F0695B, 3},
        {"バク", 0x04F759CD, 3},
        {"ハク", 0x9DFE0877, 3},
        {"でんぱく小僧", 0xEAF938E1, 3},
        {"でんじん", 0x7A462570, 3},
        {"はつでんしん", 0x0D4115E6, 3},
        {"うんがい鏡", 0x6D869C03, 3},
        {"ヤミ鏡", 0x1A81AC95, 3},
        {"まぼ老師", 0x8388FD2F, 3},
        {"かげ老師", 0xF48FCDB9, 3},
        {"ほう老師", 0x6AEB581A, 3},
        {"ゆきおんな", 0x1DEC688C, 3},
        {"ふぶき姫", 0x84E53936, 3},
        {"百鬼姫", 0xF3E209A0, 3},
        {"キュウビ", 0x635D1431, 3},
        {"犬神", 0x145A24A7, 3},
        {"だるだるま", 0x46ABCFC0, 3},
        {"だるまっちょ", 0x31ACFF56, 3},
        {"ゴリだるま", 0xA8A5AEEC, 3},
        {"イガイガグリ", 0xDFA29E7A, 3},
        {"チクチクウニ", 0x41C60BD9, 3},
        {"しゃれこ婦人", 0x36C13B4F, 3},
        {"カラカラさん", 0xAFC86AF5, 3},
        {"ほね美人", 0xD8CF5A63, 3},
        {"ムリカベ", 0x487047F2, 3},
        {"むりだ城", 0x3F777764, 3},
        {"シロカベ", 0x5FB0FE81, 3},
        {"トオセンボン", 0x28B7CE17, 3},
        {"くろがねセンボン", 0xB1BE9FAD, 3},
        {"ヨロイさん", 0xC6B9AF3B, 3},
        {"ふじのやま", 0x58DD3A98, 3},
        {"さくらのじま", 0x2FDA0A0E, 3},
        {"武者かぶと", 0xB6D35BB4, 3},
        {"かぶと無双", 0xC1D46B22, 3},
        {"オオツノノ神", 0x516B76B3, 3},
        {"キンカク", 0x266C4625, 3},
        {"ドウカク", 0xD828E24E, 3},
        {"ギンカク", 0xAF2FD2D8, 3},
        {"プラチナカク", 0x36268362, 3},
        {"だいだらぼっち", 0x4121B3F4, 3},
        {"うみぼうず", 0xDF452657, 3},
        {"セミまる", 0xA84216C1, 3},
        {"カゲまる", 0x314B477B, 3},
        {"ヒグラシまる", 0x464C77ED, 3},
        {"トホホギス", 0xD6F36A7C, 3},
        {"ブカッコウ", 0xA1F45AEA, 3},
        {"死神鳥", 0xC133D30F, 3},
        {"ジバニャン", 0xB634E399, 3},
        {"トゲニャン", 0x2F3DB223, 3},
        {"ワルニャン", 0x583A82B5, 3},
        {"ロボニャン", 0xC65E1716, 3},
        {"ゴルニャン", 0xB1592780, 3},
        {"ダイヤニャン", 0x2850763A, 3},
        {"サファイニャン", 0x5F5746AC, 3},
        {"エメラルニャン", 0xCFE85B3D, 3},
        {"ルビーニャン", 0xB8EF6BAB, 3},
        {"トパニャン", 0x11330271, 3},
        {"ノガッパ", 0x663432E7, 3},
        {"たびガッパ", 0xFF3D635D, 3},
        {"なみガッパ", 0x883A53CB, 3},
        {"コマさん", 0x165EC668, 3},
        {"コマじろう", 0x6159F6FE, 3},
        {"ししコマ", 0xF850A744, 3},
        {"とらじろう", 0x8F5797D2, 3},
        {"ボー坊", 0x1FE88A43, 3},
        {"しどろもどろ", 0x68EFBAD5, 3},
        {"さむガリ", 0x08283330, 3},
        {"ガリ王子", 0x7F2F03A6, 3},
        {"あつガルル", 0xE626521C, 3},
        {"モレゾウ", 0x9121628A, 3},
        {"プルファント", 0x0F45F729, 3},
        {"さいの目入道", 0x7842C7BF, 3},
        {"ゾロ目大明神", 0xE14B9605, 3},
        {"天狗", 0x964CA693, 3},
        {"ほむら天狗", 0x06F3BB02, 3},
        {"ひも爺", 0x71F48B94, 3},
        {"こめ爺", 0x230560F3, 3},
        {"くいい爺", 0x54025065, 3},
        {"じんめん犬", 0xCD0B01DF, 3},
        {"かおベロス", 0xBA0C3149, 3},
        {"三途の犬", 0x2468A4EA, 3},
        {"笑ウツボ", 0x536F947C, 3},
        {"歌ウナギ", 0xCA66C5C6, 3},
        {"大蛇のツボ", 0xBD61F550, 3},
        {"ぜっこう蝶", 0x2DDEE8C1, 3},
        {"ゼッコウ蝶", 0x5AD9D857, 3},
        {"サイコウ蝶", 0x3A1E51B2, 3},
        {"アゲアゲハ", 0x4D196124, 3},
        {"ワカメくん", 0xD410309E, 3},
        {"コンブさん", 0xA3170008, 3},
        {"メカブちゃん", 0x3D7395AB, 3},
        {"キュン太郎", 0x4A74A53D, 3},
        {"ズキュキュン太", 0xD37DF487, 3},
        {"裏キュン太", 0xA47AC411, 3},
        {"ホノボーノ", 0x34C5D980, 3},
        {"ヒョウヘンヌ", 0x43C2E916, 3},
        {"ヒョウヘンナ", 0x755FC775, 3},
        {"サンタク老師", 0x0258F7E3, 3},
        {"ふくろじじい", 0x9B51A659, 3},
        {"聖オカン", 0xEC5696CF, 3},
        {"心オバア", 0x7232036C, 3},
        {"雷オトン", 0x053533FA, 3},
        {"絶オジイ", 0x9C3C6240, 3},
        {"かたのり小僧", 0xEB3B52D6, 3},
        {"かたのり親方", 0x7B844F47, 3},
        {"ぎっくり男", 0x0C837FD1, 3},
        {"ネガティブーン", 0x6C44F634, 3},
        {"ジャネガブーン", 0x1B43C6A2, 3},
        {"かゆかゆ", 0x824A9718, 3},
        {"みちび鬼", 0xF54DA78E, 3},
        {"えこひい鬼", 0x6B29322D, 3},
        {"ヨコドリ", 0x1C2E02BB, 3},
        {"はらおドリ", 0x85275301, 3},
        {"口だけおんな", 0xF2206397, 3},
        {"おつぼね様", 0x629F7E06, 3},
        {"グレるりん", 0x15984E90, 3},
        {"ゴクドー", 0x4769A5F7, 3},
        {"アニ鬼", 0x306E9561, 3},
        {"魔ガサス", 0xA967C4DB, 3},
        {"ウ魔", 0xDE60F44D, 3},
        {"しわくちゃん", 0x400461EE, 3},
        {"老いらん", 0x37035178, 3},
        {"ふじみ御前", 0xAE0A00C2, 3},
        {"おならず者", 0xD90D3054, 3},
        {"へこ鬼神", 0x49B22DC5, 3},
        {"おすべり様", 0x3EB51D53, 3},
        {"おしっしょう", 0x5E7294B6, 3},
        {"フゥミン", 0x2975A420, 3},
        {"おねむの精", 0xB07CF59A, 3},
        {"ジミー", 0xC77BC50C, 3},
        {"カゲロー", 0x591F50AF, 3},
        {"カイム", 0x2E186039, 3},
        {"じめりんぼう", 0xB7113183, 3},
        {"こおりんぼう", 0xC0160115, 3},
        {"たらりん", 0x50A91C84, 3},
        {"雨ふらし", 0x27AE2C12, 3},
        {"化けあられ", 0xD9EA8879, 3},
        {"ドンヨリーヌ", 0xAEEDB8EF, 3},
        {"うらやましろう", 0x37E4E955, 3},
        {"つまみぐいのすけ", 0x40E3D9C3, 3},
        {"ヒキコウモリ", 0xDE874C60, 3},
        {"トジコウモリ", 0xA9807CF6, 3},
        {"ヤドコウモリ", 0x30892D4C, 3},
        {"びんボーイ", 0x478E1DDA, 3},
        {"らくてん童", 0xD731004B, 3},
        {"ネクラマテング", 0xA03630DD, 3},
        {"ヨミテング", 0xC0F1B938, 3},
        {"ぎしんあん鬼", 0xB7F689AE, 3},
        {"ガ鬼", 0x2EFFD814, 3},
        {"あまのじゃく", 0x59F8E882, 3},
        {"デビビル", 0xC79C7D21, 3},
        {"デビビラン", 0xB09B4DB7, 3},
        {"虫歯伯爵", 0x29921C0D, 3},
        {"ドケチング", 0x5E952C9B, 3},
        {"ヤミまろ", 0xCE2A310A, 3},
        {"ツチノコ", 0xB92D019C, 3},
        {"ツチノコパンダ", 0x1375BC28, 3},
        {"フクリュウ", 0x64728CBE, 3},
        {"りゅーくん", 0xFD7BDD04, 3},
        {"龍神", 0x8A7CED92, 3},
        {"青龍", 0x14187831, 3},
        {"キライギョ", 0x631F48A7, 3},
        {"ゾウオ", 0xFA16191D, 3},
        {"ギャクジョウオ", 0x8D11298B, 3},
        {"ミチクサメ", 0x1DAE341A, 3},
        {"トーシロザメ", 0x6AA9048C, 3},
        {"ムダヅカイ", 0x0A6E8D69, 3},
        {"はらわシェル", 0x7D69BDFF, 3},
        {"ナガバナ", 0xE460EC45, 3},
        {"ナガバナナ", 0x9367DCD3, 3},
        {"しきるん蛇", 0x0D034970, 3},
        {"まむし行司", 0x7A0479E6, 3},
        {"すねスネーク", 0xE30D285C, 3},
        {"オロチ", 0x940A18CA, 3},
        {"影オロチ", 0x04B5055B, 3},
        {"ブシニャン", 0x73B235CD, 3},
        {"しゅらコマ", 0x2143DEAA, 3},
        {"イケメン犬", 0x5644EE3C, 3},
        {"花さか爺", 0xCF4DBF86, 3},
        {"山吹鬼", 0xB84A8F10, 3},
        {"獅子まる", 0x262E1AB3, 3},
        {"かたづ家来", 0x51292A25, 3},
        {"おこ武者", 0xC8207B9F, 3},
        {"おにぎり侍", 0xBF274B09, 3},
        {"あかなめ", 0x2F985698, 3},
        {"あかなめ・怪", 0x589F660E, 3},
        {"フユニャン", 0x3858EFEB, 3},
        {"泥田坊", 0x4F5FDF7D, 3},
        {"泥田坊・怪", 0xD6568EC7, 3},
        {"ブリー隊長", 0xA151BE51, 3},
        {"アライ魔将", 0x3F352BF2, 3},
        {"万尾獅子", 0x48321B64, 3},
        {"焼きおに斬り", 0xD13B4ADE, 3},
        {"鬼食い", 0xA63C7A48, 3},
        {"いのちとり", 0x368367D9, 3},
        {"さかさっ傘", 0x4184574F, 3},
        {"こえんら", 0x7719792C, 3},
        {"一旦ゴメン", 0x001E49BA, 3},
        {"モノマネキン", 0x99171800, 3},
        {"Ｕ．Ｓ．Ｏ．", 0xEE102896, 3},
        {"メゾン・ドワスレ", 0x7074BD35, 3},
        {"えんらえんら", 0x07738DA3, 3},
        {"えんらえんら・怪", 0x9E7ADC19, 3},
        {"おでんじん", 0xE97DEC8F, 3},
        {"あやまり倒し", 0x79C2F11E, 3},
        {"うんがい三面鏡", 0x0EC5C188, 3},
        {"ぬえ", 0x6E02486D, 3},
        {"風魔猿", 0x190578FB, 3},
        {"ドキ土器", 0x800C2941, 3},
        {"うらや飯", 0xF70B19D7, 3},
        {"あせっか鬼", 0x696F8C74, 3},
        {"雷蔵", 0x1E68BCE2, 3},
        {"迷い車", 0x8761ED58, 3},
        {"泥ボックン", 0xF066DDCE, 3},
        {"プライ丼", 0x60D9C05F, 3},
        {"だっせんしゃ", 0x17DEF0C9, 3},
        {"百々目鬼", 0x452F1BAE, 3},
        {"きらめ鬼", 0x32282B38, 3},
        {"肉くいおとこ", 0xAB217A82, 3},
        {"ヒライ神", 0xDC264A14, 3},
        {"ガマンモス", 0x4242DFB7, 3},
        {"大ガマ", 0x3545EF21, 3},
        {"大やもり", 0xAC4CBE9B, 3},
        {"のっぺら坊", 0xDB4B8E0D, 3},
        {"一つ目小僧", 0x4BF4939C, 3},
        {"びきゃく", 0x3CF3A30A, 3},
        {"ばくそく", 0x5C342AEF, 3},
        {"雨女", 0x2B331A79, 3},
        {"つらがわり", 0xB23A4BC3, 3},
        {"おバク様", 0xC53D7B55, 3},
        {"河童", 0x5B59EEF6, 3},
        {"水虎", 0x2C5EDE60, 3},
        {"河童・怪", 0xB5578FDA, 3},
        {"マスターニャーダ", 0xC250BF4C, 3},
        {"キズナメコ", 0x52EFA2DD, 3},
        {"から傘お化け", 0x25E8924B, 3},
        {"から傘お化け・怪", 0xDBAC3620, 3},
        {"砂夫", 0xACAB06B6, 3},
        {"どんちゃん", 0x35A2570C, 3},
        {"くだん", 0x42A5679A, 3},
        {"くだん・怪", 0xDCC1F239, 3},
        {"ざしきわらし", 0xABC6C2AF, 3},
        {"ざしきわらし・怪", 0x32CF9315, 3},
        {"晴れ男", 0x45C8A383, 3},
        {"ワカメ☆スター", 0xD577BE12, 3},
        {"キズナース", 0xA2708E84, 3},
        {"大山砂夫", 0xC2B70761, 3},
        {"から傘魔人", 0xB5B037F7, 3},
        {"ざしきわら神", 0x2CB9664D, 3},
        {"お金ナイダー", 0x5BBE56DB, 3},
        {"大くだん", 0xC5DAC378, 3},
        {"麒麟", 0xB2DDF3EE, 3},
        {"イッカク", 0x2BD4A254, 3},
        {"化け草履", 0x5CD392C2, 3},
        {"ＴＥＴＳＵＹＡ", 0xCC6C8F53, 3},
        {"かりパックン", 0xBB6BBFC5, 3},
        {"草くいおとこ", 0x12B7D61F, 3},
        {"まてんし", 0x65B0E689, 3},
        {"ひとまか仙人", 0xFCB9B733, 3},
        {"りもこんかくし", 0x8BBE87A5, 3},
        {"ミツマタノヅチ", 0x15DA1206, 3},
        {"リー夫人", 0x62DD2290, 3},
        {"ガブニャン", 0xFBD4732A, 3},
        {"ＫＡＮＴＥＴＳＵ", 0x8CD343BC, 3},
        {"どんどろ", 0x1C6C5E2D, 3},
        {"けうけげん", 0x6B6B6EBB, 3},
        {"けうけげん・怪", 0x0BACE75E, 3},
        {"認ＭＥＮ", 0x7CABD7C8, 3},
        {"じがじぃさん", 0xE5A28672, 3},
        {"花子さん", 0x92A5B6E4, 3},
        {"のらりくらり", 0x0CC12347, 3},
        {"ぶんぶん鳥", 0x7BC613D1, 3},
        {"のぼせトンマン", 0xE2CF426B, 3},
        {"爆音ならし", 0x95C872FD, 3},
        {"ふさふさん", 0x05776F6C, 3},
        {"呪野花子", 0x72705FFA, 3},
        {"土蜘蛛", 0x2081B49D, 3},
        {"女郎蜘蛛", 0x5786840B, 3},
        {"ケマモン", 0xCE8FD5B1, 3},
        {"ねちがえる", 0xB988E527, 3},
        {"からみぞん", 0x27EC7084, 3},
        {"おともらし", 0x50EB4012, 3},
        {"おもいだスッポン", 0xC9E211A8, 3},
        {"つられたろう丸", 0xBEE5213E, 3},
        {"ムカムカデ", 0x2E5A3CAF, 3},
        {"にんぎょ", 0x595D0C39, 3},
        {"にんぎょ・怪", 0x399A85DC, 3},
        {"ろくろ首", 0x4E9DB54A, 3},
        {"ろくろ首・怪", 0xD794E4F0, 3},
        {"ホリュウ", 0xA093D466, 3},
        {"激ドラゴン", 0x3EF741C5, 3},
        {"おもいだ神", 0x49F07153, 3},
        {"こんがらギャル", 0xD0F920E9, 3},
        {"八百比丘尼", 0xA7FE107F, 3},
        {"イザナミ", 0x37410DEE, 3},
        {"ネタバレリーナ", 0x40463D78, 3},
        {"うんちく魔", 0x76DB131B, 3},
        {"やまタン", 0x01DC238D, 3},
        {"厄怪", 0x98D57237, 3},
        {"不怪", 0xEFD242A1, 3},
        {"豪怪", 0x71B6D702, 3},
        {"難怪", 0x06B1E794, 3},
        {"破怪", 0x9FB8B62E, 3},
        {"ジコチュウ", 0xE8BF86B8, 3},
        {"アペリカン", 0x78009B29, 3},
        {"ダークニャン", 0x0F07ABBF, 3},
        {"ブチニャン", 0x6FC0225A, 3},
        {"セバスチャン", 0x18C712CC, 3},
        {"ロボニャンＦ型", 0x81CE4376, 3},
        {"セーラーニャン", 0xF6C973E0, 3},
        {"マスクドニャーン", 0x68ADE643, 3},
        {"ジバコマ", 0x1FAAD6D5, 3},
        {"ジンギスギスカン", 0x86A3876F, 3},
        {"チンギスギスハン", 0xF1A4B7F9, 3},
        {"寝ブタ", 0x611BAA68, 3},
        {"ねぶた", 0x161C9AFE, 3},
        {"江戸っ子パンダ", 0x44ED7199, 3},
        {"大江戸忍者パンダ", 0x33EA410F, 3},
        {"ひつま武士", 0xAAE310B5, 3},
        {"ひまつ武士", 0xDDE42023, 3},
        {"たこやっ鬼", 0x4380B580, 3},
        {"たこや鬼神", 0x34878516, 3},
        {"砂ン丘入道", 0xAD8ED4AC, 3},
        {"ノー砂ン丘入道", 0xDA89E43A, 3},
        {"すもうどん", 0x4A36F9AB, 3},
        {"横綱うどん", 0x3D31C93D, 3},
        {"あっそう山", 0x5DF640D8, 3},
        {"あそっ火山", 0x2AF1704E, 3},
        {"ウキウキビ", 0xB3F821F4, 3},
        {"トキメキビ", 0xC4FF1162, 3},
        {"ロボガッパ", 0x5A9B84C1, 3},
        {"ロボコマ", 0x2D9CB457, 3},
        {"ロボＧ", 0xB495E5ED, 3},
        {"ロボメン犬", 0xC392D57B, 3},
        {"ロボノコ", 0x532DC8EA, 3},
        {"ロボりゅーくん", 0x242AF87C, 3},
        {"メロンニャン", 0xDA6E5C17, 3},
        {"ミカンニャン", 0xAD696C81, 3},
        {"キウイニャン", 0x34603D3B, 3},
        {"ブドウニャン", 0x43670DAD, 3},
        {"イチゴニャン", 0xDD03980E, 3},
        {"スイカニャン", 0xAA04A898, 3},
        {"ジェットニャン", 0x330DF922, 3},
        {"ワンダーニャン", 0x440AC9B4, 3},
        {"おんみつ魂", 0xA84D0F23, 3},
        {"スパルタ魂", 0xDF4A3FB5, 3},
        {"壺ガード魂", 0x46436E0F, 3},
        {"ガード魂", 0x31445E99, 3},
        {"かんつう魂", 0xAF20CB3A, 3},
        {"ちょうはつ魂", 0xD827FBAC, 3},
        {"閃光魂", 0x412EAA16, 3},
        {"なめらかオイル魂", 0x36299A80, 3},
        {"こじらせ魂", 0xA6968711, 3},
        {"れんごく魂", 0xD191B787, 3},
        {"大滝魂", 0xB1563E62, 3},
        {"雷神魂", 0xC6510EF4, 3},
        {"いん石魂", 0x5F585F4E, 3},
        {"吹雪魂", 0x285F6FD8, 3},
        {"嵐魂", 0xB63BFA7B, 3},
        {"火の魂", 0x58359B57, 3},
        {"水の魂", 0x2F32ABC1, 3},
        {"雷の魂", 0xBF8DB650, 3},
        {"土の魂", 0xC88A86C6, 3},
        {"氷の魂", 0x9A7B6DA1, 3},
        {"風の魂", 0xED7C5D37, 3},
        {"ヤミ魂", 0x74750C8D, 3},
        {"黄泉魂", 0x03723C1B, 3},
        {"ブロッカー魂", 0x9D16A9B8, 3},
        {"よびよせ魂", 0xEA11992E, 3},
        {"無の魂", 0x7318C894, 3},
        {"モテモテ魂", 0x041FF802, 3},
        {"ミンミンゼミ", 0xD03E27FE, 4},
        {"ミンミンゼミ★", 0x6882409B, 4},
        {"アブラゼミ", 0x49377644, 4},
        {"アブラゼミ★", 0xF18B1121, 4},
        {"ツクツクボウシ", 0x6577E19E, 4},
        {"ツクツクボウシ★", 0xDDCB86FB, 4},
        {"ニイニイゼミ", 0xFC7EB024, 4},
        {"ニイニイゼミ★", 0x44C2D741, 4},
        {"ヒグラシ", 0x3E3046D2, 4},
        {"ヒグラシ★", 0x868C21B7, 4},
        {"クマゼミ", 0x8B7980B2, 4},
        {"クマゼミ★", 0x33C5E7D7, 4},
        {"ノコギリクワガタ", 0x4E5AB25D, 4},
        {"ノコギリクワガタ★", 0xF6E6D538, 4},
        {"ヒラタクワガタ", 0x332D4618, 4},
        {"ヒラタクワガタ★", 0x8B91217D, 4},
        {"ミヤマクワガタ", 0x151D1511, 4},
        {"ミヤマクワガタ★", 0xADA17274, 4},
        {"オオクワガタ", 0xD753E3E7, 4},
        {"オオクワガタ★", 0x6FEF8482, 4},
        {"カブトムシ", 0xA054D371, 4},
        {"カブトムシ★", 0x18E8B414, 4},
        {"モンシロチョウ", 0x8C1444AB, 4},
        {"モンシロチョウ★", 0x34A823CE, 4},
        {"アゲハチョウ", 0xC92516BF, 4},
        {"アゲハチョウ★", 0x719971DA, 4},
        {"カラスアゲハ", 0x7B0114C6, 4},
        {"カラスアゲハ★", 0xC3BD73A3, 4},
        {"オオムラサキ", 0x2046B38A, 4},
        {"オオムラサキ★", 0x98FAD4EF, 4},
        {"シオカラトンボ", 0xDA4EE32D, 4},
        {"シオカラトンボ★", 0x62F28448, 4},
        {"ギンヤンマ", 0xAD49D3BB, 4},
        {"ギンヤンマ★", 0x15F5B4DE, 4},
        {"オニヤンマ", 0xB0F9AE1B, 4},
        {"オニヤンマ★", 0x0845C97E, 4},
        {"ホタル", 0xBE222629, 4},
        {"ホタル★", 0x069E414C, 4},
        {"テントウムシ", 0xB94FE230, 4},
        {"テントウムシ★", 0x01F38555, 4},
        {"ショウリョウバッタ", 0x395D82CB, 4},
        {"ショウリョウバッタ★", 0x81E1E5AE, 4},
        {"イナゴ", 0x5741831C, 4},
        {"イナゴ★", 0xEFFDE479, 4},
        {"トノサマバッタ", 0xDEE5AFCC, 4},
        {"トノサマバッタ★", 0x6659C8A9, 4},
        {"コオロギ", 0x9262B1F3, 4},
        {"コオロギ★", 0x2ADED696, 4},
        {"スズムシ", 0x7C6CD0DF, 4},
        {"スズムシ★", 0xC4D0B7BA, 4},
        {"キリギリス", 0x0C062450, 4},
        {"キリギリス★", 0xB4BA4335, 4},
        {"ハラビロカマキリ", 0x5D3147CF, 4},
        {"ハラビロカマキリ★", 0xE58D20AA, 4},
        {"オオカマキリ", 0xA9E29F5A, 4},
        {"オオカマキリ★", 0x115EF83F, 4},
        {"ゴマダラカミキリ", 0x502C4705, 4},
        {"ゴマダラカミキリ★", 0xE8902060, 4},
        {"ナナフシ", 0xCD8E5A5E, 4},
        {"ナナフシ★", 0x75323D3B, 4},
        {"オケラ", 0xDD232734, 4},
        {"オケラ★", 0x659F4051, 4},
        {"ダンゴムシ", 0x9BD4FDD8, 4},
        {"ダンゴムシ★", 0x23689ABD, 4},
        {"ハナムグリ", 0x272B7793, 4},
        {"ハナムグリ★", 0x9F9710F6, 4},
        {"コガネムシ", 0xE5658165, 4},
        {"コガネムシ★", 0x5DD9E600, 4},
        {"カナブン", 0x950F75EA, 4},
        {"カナブン★", 0x2DB3128F, 4},
        {"カメムシ", 0xE208457C, 4},
        {"カメムシ★", 0x5AB42219, 4},
        {"ゲンゴロウ", 0xF5C8FC0F, 4},
        {"ゲンゴロウ★", 0x4D749B6A, 4},
        {"タガメ", 0xB452E2FA, 4},
        {"タガメ★", 0x0CEE859F, 4},
        {"タマムシ", 0x0B6BE049, 4},
        {"タマムシ★", 0xB3D7872C, 4},
        {"ニホントカゲ", 0xECD3CD4E, 4},
        {"ニホントカゲ★", 0x546FAA2B, 4},
        {"カナヘビ", 0xAA2417A2, 4},
        {"カナヘビ★", 0x129870C7, 4},
        {"アカハライモリ", 0x82CFCC99, 4},
        {"アカハライモリ★", 0x3A73ABFC, 4},
        {"カブトエビ", 0x5A5C83D6, 4},
        {"カブトエビ★", 0xE2E0E4B3, 4},
        {"タニシ", 0x2A367759, 4},
        {"タニシ★", 0x928A103C, 4},
        {"ジャンボタニシ", 0x442A768E, 4},
        {"ジャンボタニシ★", 0xFC9611EB, 4},
        {"カタツムリ", 0xC7FE9E8D, 4},
        {"カタツムリ★", 0x7F42F9E8, 4},
        {"アマガエル", 0xCE48D2A6, 4},
        {"アマガエル★", 0x76F4B5C3, 4},
        {"ヒキガエル", 0xFB13743D, 4},
        {"ヒキガエル★", 0x43AF1358, 4},
        {"カメ", 0xD1FC4DC9, 4},
        {"カメ★", 0x69402AAC, 4},
        {"スッポン", 0xC63CF4BA, 4},
        {"スッポン★", 0x7E8093DF, 4},
        {"サワガニ", 0x0DC44E67, 4},
        {"サワガニ★", 0xB5782902, 4},
        {"アメリカザリガニ", 0x94CD1FDD, 4},
        {"アメリカザリガニ★", 0x2C7178B8, 4},
        {"イセエビ", 0x7DAEBAE8, 4},
        {"イセエビ★", 0xC512DD8D, 4},
        {"シャコ", 0xC5FA7C42, 4},
        {"シャコ★", 0x7D461B27, 4},
        {"ヒトデ", 0xB2FD4CD4, 4},
        {"ヒトデ★", 0x0A412BB1, 4},
        {"タツノオトシゴ", 0x14DF7F26, 4},
        {"タツノオトシゴ★", 0xAC631843, 4},
        {"フナ", 0xA196B946, 4},
        {"フナ★", 0x192ADE23, 4},
        {"アユ", 0xD69189D0, 4},
        {"アユ★", 0x6E2DEEB5, 4},
        {"ボラ", 0x2C99D977, 4},
        {"ボラ★", 0x9425BE12, 4},
        {"ヤマメ", 0x93A0DBC4, 4},
        {"ヤマメ★", 0x2B1CBCA1, 4},
        {"コイ", 0xB13BC42C, 4},
        {"コイ★", 0x0987A349, 4},
        {"ブラックバス", 0x48F51C73, 4},
        {"ブラックバス★", 0xF0497B16, 4},
        {"サケ", 0x7AC37EF1, 4},
        {"サケ★", 0xC27F1994, 4},
        {"イトウ", 0x3FF22CE5, 4},
        {"イトウ★", 0x874E4B80, 4},
        {"イワシ", 0x0AA98A7E, 4},
        {"イワシ★", 0xB215ED1B, 4},
        {"アジ", 0x389FE8FC, 4},
        {"アジ★", 0x80238F99, 4},
        {"サバ", 0xFAD11E0A, 4},
        {"サバ★", 0x426D796F, 4},
        {"キス", 0x8DD62E9C, 4},
        {"キス★", 0x356A49F9, 4},
        {"カマス", 0xDB8C891A, 4},
        {"カマス★", 0x6330EE7F, 4},
        {"トビウオ", 0x8ABBEA85, 4},
        {"トビウオ★", 0x32078DE0, 4},
        {"ライギョ", 0x4F98D86A, 4},
        {"ライギョ★", 0xF724BF0F, 4},
        {"カサゴ", 0xCF8AB891, 4},
        {"カサゴ★", 0x7736DFF4, 4},
        {"イサキ", 0x5B9EE9E1, 4},
        {"イサキ★", 0xE3228E84, 4},
        {"スズキ", 0x5CF32DF8, 4},
        {"スズキ★", 0xE44F4A9D, 4},
        {"メジナ", 0xABE67D95, 4},
        {"メジナ★", 0x135A1AF0, 4},
        {"カワハギ", 0xED11A779, 4},
        {"カワハギ★", 0x55ADC01C, 4},
        {"ヒラメ", 0x64B58BA9, 4},
        {"ヒラメ★", 0xDC09ECCC, 4},
        {"カレイ", 0xC297B85B, 4},
        {"カレイ★", 0x7A2BDF3E, 4},
        {"タイ", 0xA820F56D, 4},
        {"タイ★", 0x109C9208, 4},
        {"イシダイ", 0x5683E92B, 4},
        {"イシダイ★", 0xEE3F8E4E, 4},
        {"チヌ", 0xDCE14D03, 4},
        {"チヌ★", 0x645D2A66, 4},
        {"ブリ", 0x13B2BB3F, 4},
        {"ブリ★", 0xAB0EDC5A, 4},
        {"ハリセンボン", 0xFDBCDA13, 4},
        {"ハリセンボン★", 0x4500BD76, 4},
        {"ドジョウ", 0x2184D9BD, 4},
        {"ドジョウ★", 0x9938BED8, 4},
        {"タチウオ", 0x2BF41D6E, 4},
        {"タチウオ★", 0x93487A0B, 4},
        {"アナゴ", 0xE4A7EB52, 4},
        {"アナゴ★", 0x5C1B8C37, 4},
        {"ウナギ", 0xE3CA2F4B, 4},
        {"ウナギ★", 0x5B76482E, 4},
        {"ナマズ", 0xB88D8807, 4},
        {"ナマズ★", 0x0031EF62, 4},
        {"クラゲ", 0xC8E77C88, 4},
        {"クラゲ★", 0x705B1BED, 4},
        {"タコ", 0x63D84FB0, 4},
        {"タコ★", 0xDB6428D5, 4},
        {"モイカ", 0xAC8BB98C, 4},
        {"モイカ★", 0x1437DEE9, 4},
        {"コウイカ", 0xF40A9638, 4},
        {"コウイカ★", 0x4CB6F15D, 4},
        {"ホタルイカ", 0x51EE2D32, 4},
        {"ホタルイカ★", 0xE9524A57, 4},
        {"スルメイカ", 0xB59088CD, 4},
        {"スルメイカ★", 0x0D2CEFA8, 4},
        {"ヤリイカ", 0x3582E836, 4},
        {"ヤリイカ★", 0x8D3E8F53, 4},
        {"チョウチンアンコウ", 0x4285D8A0, 4},
        {"チョウチンアンコウ★", 0xFA39BFC5, 4},
        {"エイ", 0x9A1697EF, 4},
        {"エイ★", 0x22AAF08A, 4},
        {"カジキ", 0xBFE04C1E, 4},
        {"カジキ★", 0x075C2B7B, 4},
        {"マグロ", 0x830DA6AE, 4},
        {"マグロ★", 0x3BB1C1CB, 4},
    };
    static const u32 g_itemTableCount = sizeof(g_items) / sizeof(g_items[0]);
    static std::string g_itemName;

    typedef void (*AddItemFn)(u32 obj, u32 id, u32 count);
    static const u32 ADD_ITEM_FN  = 0x005DD738;
    static const u32 ADD_ITEM_OBJ = 0x0080D6BE;

    static u32  g_itemId      = 0;
    static u32  g_itemCount   = 1;
    static bool g_itemOnStart = false;

    static bool CreateItem(void)
    {
        u32 probe = 0;
        if (g_itemId == 0 || !Process::Read32(ADD_ITEM_FN, probe) || probe == 0)
            return false;
        ((AddItemFn)ADD_ITEM_FN)(ADD_ITEM_OBJ, g_itemId, g_itemCount);
        return true;
    }

    // 毎フレーム呼ばれる。「Startで作成」がONなら、Startを押した瞬間に1回だけ作る
    static void ItemFrame(MenuEntry *entry)
    {
        if (g_itemOnStart && Controller::IsKeyPressed((Key)0x08))
        {
            bool ok = CreateItem();
            OSD::Notify(ok ? "アイテムを作成: " + Hex8(g_itemId) + " x" + Dec(g_itemCount)
                           : std::string("アイテム作成: IDを設定してください"));
        }
    }

    static void PickItem(void)
    {
        std::vector<std::string> cats = {"消耗品", "大事なもの", "装備品", "ソウル", "妖怪(IDのみ・作成不可の可能性)"};
        Keyboard ck("種類", cats);
        int c = ck.Open();
        if (c < 0)
            return;

        std::vector<u32>         idx;
        std::vector<std::string> names;
        for (u32 i = 0; i < g_itemTableCount; i++)
        {
            if (g_items[i].cat == c)
            {
                idx.push_back(i);
                names.push_back(g_items[i].name);
            }
        }
        Keyboard lk("アイテムを選択", names);
        int s = lk.Open();
        if (s < 0 || s >= (int)idx.size())
            return;
        g_itemId   = g_items[idx[s]].id;
        g_itemName = g_items[idx[s]].name;
        OSD::Notify("アイテム: " + g_itemName);
    }

    static void ItemMenu(MenuEntry *entry)
    {
        for (;;)
        {
            std::vector<std::string> opts =
            {
                "アイテムを一覧から選ぶ",
                "選択中: " + (g_itemId ? (g_itemName.empty() ? Hex8(g_itemId) : TruncU8(g_itemName, 16)) : std::string("(未設定)")),
                "アイテムIDを直接入力(16進)",
                "個数: " + Dec(g_itemCount),
                "今すぐ作成",
                std::string("Startボタンで作成: ") + (g_itemOnStart ? "ON" : "OFF"),
            };
            Keyboard kb("アイテム作成", opts);
            int r = kb.Open();
            if (r < 0)
                return;

            if (r == 0)
                PickItem();
            else if (r == 1)
            {
                ShowInfo("アイテム: " + (g_itemName.empty() ? std::string("(名前なし)") : g_itemName) + "\n"
                         "ID: " + (g_itemId ? Hex8(g_itemId) : std::string("(未設定)")));
            }
            else if (r == 2)
            {
                u32 v = g_itemId;
                Keyboard hex("アイテムID(16進)");
                hex.IsHexadecimal(true);
                if (hex.Open(v, g_itemId) >= 0)
                {
                    g_itemId = v;
                    g_itemName = "";
                    for (u32 i = 0; i < g_itemTableCount; i++)
                        if (g_items[i].id == v)
                            g_itemName = g_items[i].name;
                }
            }
            else if (r == 3)
            {
                u32 v = g_itemCount;
                Keyboard ck("個数");
                if (ck.Open(v, g_itemCount) >= 0 && v > 0 && v < 10000)
                    g_itemCount = v;
            }
            else if (r == 4)
            {
                if (g_itemId == 0)
                {
                    ShowInfo("先にアイテムを選んでください。");
                    continue;
                }
                std::vector<std::string> yn = {"作成する", "やめる"};
                Keyboard ak((g_itemName.empty() ? Hex8(g_itemId) : g_itemName) + " を " + Dec(g_itemCount) + "個 作成します。\n"
                            "このゲーム(真打)のコードなので、別のバージョンでは固まることがあります。", yn);
                if (ak.Open() == 0)
                {
                    bool ok = CreateItem();
                    OSD::Notify(ok ? "アイテムを作成しました" : "作成できませんでした");
                }
            }
            else if (r == 5)
                g_itemOnStart = !g_itemOnStart;
        }
    }

    // ================= 座標表示 =================
    static bool CoordOSD(const Screen &screen)
    {
        if (!g_showCoords || !screen.IsTop)
            return false;

        float x = 0.0f, y = 0.0f, z = 0.0f;
        bool ok = Process::ReadFloat(PLAYER_X_ADDR, x)
               && Process::ReadFloat(PLAYER_Y_ADDR, y)
               && Process::ReadFloat(PLAYER_Z_ADDR, z);

        char line[64];
        if (ok)
            snprintf(line, sizeof(line), "X:%.1f Y:%.1f Z:%.1f", x, y, z);
        else
            snprintf(line, sizeof(line), "座標: 読み取り失敗");
        u32 py = screen.Draw(line, 8, 8);

        u32 id = 0;
        if (Process::Read32(SlotAddr(), id))
        {
            const char *n = MapNameOf(id);
            std::string m = "マップ: " + Hex8(id) + (n ? std::string(" ") + n : std::string(""));
            screen.Draw(m, 8, py);
        }
        return true;
    }

    void PatchProcess(FwkSettings &settings)
    {
    }

    void OnProcessExit(void)
    {
    }

    int main(void)
    {
        PluginMenu *menu = new PluginMenu("妖怪ウォッチ2 真打ツール", 0, 1, 0,
            "移動系の機能とマップワープ。");

        menu->SynchronizeWithFrame(true);

        menu->Append(new MenuEntry("移動系", MoveFrame, MoveMenu,
            "座標移動・ムーンジャンプ・空中歩行・壁抜け・スタミナ・スピード"));
        menu->Append(new MenuEntry("マップワープ", MapWarpFrame, MapWarpMenu,
            "行き先を選び、ホットキーを押している間ワープする"));
        menu->Append(new MenuEntry("アイテム作成", ItemFrame, ItemMenu,
            "アイテムIDと個数を指定して作成(Startボタンでも作成できる)"));

        OSD::Run(CoordOSD);

        menu->Run();
        delete menu;
        return 0;
    }
}
