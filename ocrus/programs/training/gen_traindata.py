# coding=utf-8
'''
Generate training data

Copyright (C) 2016 Works Applications, all rights reserved
'''

import json
import io
import sys
import time
import cPickle
import gzip
import random

import cv2
import Image
import ImageFont
import ImageDraw
import numpy as np

from ocrus.util.mnist_format import remove_white_border,\
    gray_img_to_mnist_array, pretty_print_char, mnist_array_to_gray_img
from ocrus.util.convert_format import to_chars

import codecs
import locale
sys.stdout = codecs.getwriter(locale.getpreferredencoding())(sys.stdout)

SHOW_CHAR = False
SHOW_SLEEP = 0.5
SHOW_CV2_CHAR = False
SHOW_CV2_SLEEP = 500
FONT_CHARS_CV2 = [u'0', u'1', u'￥', u'円']

SUFFIX_ID = 'full'
PATH_PKL_GZ = '/home/csuncs89/0-priceless/programming/ocrus0_build/jpn_%s.pkl.gz' % SUFFIX_ID
PATH_CHARS_ID = '/home/csuncs89/0-priceless/programming/ocrus0_build/chars_id_%s.json' % SUFFIX_ID


digit_line = u'''
0 1 2 3 4 5 6 7 8 9
'''

symbol_line = u'''
￥
'''

punc_line = u'''
/ - ,  ， 。 ' "
'''

letter_line = u'''
a b c d e f g
h i j k l m n
o p q   r s t
u v w   x y z
A B C D E F G
H I J K L M N
O P Q   R S T
U V W   X Y Z
'''

hiragana_line = u'''
 あ    い    う    え    お  か  が  き  ぎ  く
 ぐ  け  げ  こ  ご  さ  ざ  し  じ  す  ず  せ  ぜ  そ  ぞ  た
 だ  ち  ぢ    つ  づ  て  で  と  ど  な  に  ぬ  ね  の  は
 ば  ぱ  ひ  び  ぴ  ふ  ぶ  ぷ  へ  べ  ぺ  ほ  ぼ  ぽ  ま  み
 む  め  も    や    ゆ    よ  ら  り  る  れ  ろ    わ
 ゐ  ゑ  を  ん  ゔ
'''

katakana_line = u'''
  ア    イ    ウ    エ    オ  カ  ガ  キ  ギ  ク
グ  ケ  ゲ  コ  ゴ  サ  ザ  シ  ジ  ス  ズ  セ  ゼ  ソ  ゾ  タ
ダ  チ  ヂ    ツ  ヅ  テ  デ  ト  ド  ナ  ニ  ヌ  ネ  ノ  ハ
バ  パ  ヒ  ビ  ピ  フ  ブ  プ  ヘ  ベ  ペ  ホ  ボ  ポ  マ  ミ
ム  メ  モ    ヤ    ユ    ヨ  ラ  リ  ル  レ  ロ    ワ
 ヲ  ン  ヴ      ヷ   ・  ー
'''

kanji_lines = [
    u'''
一右雨円王音下火花貝学気九休玉金空月犬見五口校左三山子四糸字耳七車手十出女小上森人水正生青夕石赤千川先早草足村大男竹中虫町天田土二日入年白八百文木本名目立力林六
''',
    u'''
引羽雲園遠何科夏家歌画回会海絵外角楽活間丸岩顔汽記帰弓牛魚京強教近兄形計元言原戸古午後語工公広交光考行高黄合谷国黒今才細作算止市矢姉思紙寺自時室社弱首秋週春書少場色食心新親図数西声星晴切雪船線前組走多太体台地池知茶昼長鳥朝直通弟店点電刀冬当東答頭同道読内南肉馬売買麦半番父風分聞米歩母方北毎妹万明鳴毛門夜野友用曜来里理話
''',
    u'''
悪安暗医委意育員院飲運泳駅央横屋温化荷界開階寒感漢館岸起期客究急級宮球去橋業曲局銀区苦具君係軽血決研県庫湖向幸港号根祭皿仕死使始指歯詩次事持式実写者主守取酒受州拾終習集住重宿所暑助昭消商章勝乗植申身神真深進世整昔全相送想息速族他打対待代第題炭短談着注柱丁帳調追定庭笛鉄転都度投豆島湯登等動童農波配倍箱畑発反坂板皮悲美鼻筆氷表秒病品負部服福物平返勉放味命面問役薬由油有遊予羊洋葉陽様落流旅両緑礼列練路和
''',
    u'''
愛案以衣位囲胃印英栄塩億加果貨課芽改械害街各覚完官管関観願希季紀喜旗器機議求泣救給挙漁共協鏡競極訓軍郡径型景芸欠結建健験固功好候航康告差菜最材昨札刷殺察参産散残士氏史司試児治辞失借種周祝順初松笑唱焼象照賞臣信成省清静席積折節説浅戦選然争倉巣束側続卒孫帯隊達単置仲貯兆腸低底停的典伝徒努灯堂働特得毒熱念敗梅博飯飛費必票標不夫付府副粉兵別辺変便包法望牧末満未脈民無約勇要養浴利陸良料量輪類令冷例歴連老労録
''',
    u'''
圧移因永営衛易益液演応往桜恩可仮価河過賀快解格確額刊幹慣眼基寄規技義逆久旧居許境均禁句群経潔件券険検限現減故個護効厚耕鉱構興講混査再災妻採際在財罪雑酸賛支志枝師資飼示似識質舎謝授修述術準序招承証条状常情織職制性政勢精製税責績接設舌絶銭祖素総造像増則測属率損退貸態団断築張提程適敵統銅導徳独任燃能破犯判版比肥非備俵評貧布婦富武復複仏編弁保墓報豊防貿暴務夢迷綿輸余預容略留領
''',
    u'''
異遺域宇映延沿我灰拡革閣割株干巻看簡危机揮貴疑吸供胸郷勤筋系敬警劇激穴絹権憲源厳己呼誤后孝皇紅降鋼刻穀骨困砂座済裁策冊蚕至私姿視詞誌磁射捨尺若樹収宗就衆従縦縮熟純処署諸除将傷障城蒸針仁垂推寸盛聖誠宣専泉洗染善奏窓創装層操蔵臓存尊宅担探誕段暖値宙忠著庁頂潮賃痛展討党糖届難乳認納脳派拝背肺俳班晩否批秘腹奮並陛閉片補暮宝訪亡忘棒枚幕密盟模訳郵優幼欲翌乱卵覧裏律臨朗論
''',
    u'''
乙了又丈与及乞凡刃巾互丹乏井冗凶刈勾匂匹升厄双介孔屯幻弔斗斤爪牙且丘丙丼巨仙凹凸占叱召囚奴尻尼巧払氾汁込斥旦玄瓦甘甲矛伎仰伐伏充刑劣匠企吉叫吐吏壮如妃妄尽巡帆弐忙扱汎汚汗江芋芝迅旨旬肌朽朱朴缶臼舟串亜佐伺伸但伯伴克冶励却即呂含吟呉吹呈坑坊壱妖妥妊妨妙寿尿尾岐床廷弄抗抄択把抜扶抑沙汰沃沖沢沈没狂芯芳迎那邦阪忌忍戒戻攻更肘肝肖杉秀辛享依佳侍侮併免刹刺到劾卓叔呪坪奈奇奉奔妬姓宛宜尚屈岡岳岬弥弦征彼怪怖拉押拐拒拠拘拙拓抽抵拍披抱抹況沼泥泊泌沸泡狙苛茎苗茂迭迫邪邸阻附房旺昆昇股肩肯肢肪枕枢析杯枠欧殴炎炊炉采玩祈祉盲突虎阜斉亭侶侯俊侵促俗冠削勃勅卑卸厘叙咽哀咲垣契威姻孤封峡峠帥帝幽弧悔恒恨拶拭括挟拷挑洪浄津洞狭狩茨荒荘逃郊郎怨怠怒施昧是胎胆胞柿柵栃架枯柔柄某柳為牲珍甚畏疫皆盆眉盾冒砕窃糾耐臭虐虹衷訃訂貞赴軌香俺倹倒倣俸倫兼冥凄准凍剥剣剛剤剖匿唄哺唆唇哲唐埋娯娠姫娘宴宰宵峰徐悦悟悩挨捉挫振捜挿捕浦浸浜浮涙浪華逝逐逓途透陥陣恣恐恵恥恋恭扇拳敏脇脊脅脂朕胴桁核桑栽桟栓桃殊殉烈珠祥泰畝畜畔疾症疲眠砲称租秩袖被既粋索紛紡紋翁耗致般蚊衰託貢軒辱酎酌釜隻飢鬼竜曹乾偽偶偵偏剰勘唾喝啓唯埼堆執培婚婆寂尉崖崎崇崩庶庸彩彫惧惨惜悼捻捗掛掘掲控据措掃排描堀淫涯渇渓渋淑渉淡添涼猫猛猟葛萎菓菊菌逸逮郭陰陳陶陪隆陵患悠戚斜斬旋曽脚脱梗梨殻爽瓶痕盗眺窒符粗粘粒紺紹紳累羞粛舷舶虚蛍蛇袋訟豚貪貫販赦軟酔釈釣頃鹿麻斎亀僅偉傍募傘喉喩喚喫喪圏堪堅堕塚堤塔塀塁奥媛媒婿尋嵐項幅帽幾廃廊弾御循慌惰愉握援換搭揚揺湧渦滋湿渡湾猶葬遇遂遅遍隅随惑扉掌敢斑暁晶替普腕椅椎棺棋棚棟款欺殖煮焦琴畳疎痩痘痢硬硝硫裕筒粧絞紫絡蛮裂詠詐詔診訴貼越超距軸酢鈍閑雇雄雰須傲傾傑債催僧勧嗅嗣嘆塞填毀塊塑塗奨嫉嫁嫌寛寝廉彙微慄慨慎携搾摂搬溺滑溝滞滝漠滅溶猿蓋蓄遡遜違遣隙隔愚慈愁暇腎腫腺腰楷棄楼歳殿煙煩煎献禍禅痴睦睡督碁稚窟裾褐裸継羨艇虞虜蜂触詣詮該詰誇詳誉賂賊賄跡践跳較載酬酪鉛鉢鈴雅雷零飾飽靴頓頑頒鼓僕僚塾墨奪嫡寡寧彰徴憎慢摘漆漸漬滴漂漫漏蔑遮遭隠慕暦膜概熊獄瑠瘍罰碑稲端箸箋箇綻維綱緒網腐蜜誓誘豪貌踊辣酵酷銃銘閥雌需餅駆駄髪魂儀勲舗嘲嘱噴墜墳審寮履幣慶弊影徹憧憬憤撮撤撲潰潟潤澄潜蔽遵遷慰憂慮戯摯撃摩敷暫膝膚槽歓璃畿監盤罵罷稽稼稿穂窮窯範縁緩緊締縄衝褒誰謁請諾賭賜賠賓賦趣踪踏輝輩舞鋭鋳閲震霊餓餌頬駒駐魅黙儒凝壊墾壌壇壁嬢憶懐憾擁濁濃獲薫薪薦薄還避隣憩曇膳膨獣磨穏篤緻緯縛繁縫融衡諦謎諧諮謀諭謡賢醒麺錦錮錯錠錬隷頼骸償嚇擬擦濯懇戴曖臆燥爵犠環療瞳瞭矯礁繊翼聴謹謙謄購轄醜鍵鍋鍛闇霜韓頻鮮齢濫藤藍藩懲璧癖癒瞬礎穫襟糧繕繭翻覆贈鎌鎖鎮闘顎顕騎騒瀬藻爆璽羅簿繰艶覇譜蹴離霧韻髄鯨鶏麓麗懸欄籍譲醸鐘響騰艦躍露顧魔鶴籠襲驚鑑鬱
''',
    u'''
乃卜之也勺叉已巳丑云允匁勿廿壬尤巴乎仔凧卯叶只弘汀辻戊疋禾瓜丞亘亙亦亥伊伍凪匡圭夷尖庄弛托汝汐迂迄辿收旭曳肋此牟而亨伽佃佑伶佛冴劫吾呑吻坐壯孜宏宋庇汲沌芥芹芭芙芦杏杖杜李灸灼牡玖祁甫辰邑酉些亞來侃侑侠兎兒其函卷尭坦奄孟宕帖庚怜拔拂沫苑茄苔苺茅茉迪迦阿陀忽或斧於昂昊昏昌朋肴杭杵杷枇欣沓爭祇穹竺亮俄俣俐勁哉奎娃姪姥宥巷廻彦恢恰恆按洸洲洵洛茸茜郁拜昴胤胡柑柘柊柏柾柚殆毘珂珈珊珀玲祢祐盃穿衿竿籾耶臥頁乘倭倶倖凌凉卿哨啄哩圃套屑峨峻峽峯悌挺浩浬狼狹莞荻莫莉莱莊這逗逢恕晏晃晄晒晋晦朔柴桧栞桔桂栖桐栗氣烏畠眞矩砥砧祕秦秤窄笈紘紗紐耽訊豹赳釘閃隼倦偲兜冨凰厩圈國埴堵埜娩寅將專崚帶庵彗彪彬徠從惟惚惇挽掬捷捺捧掠掴淵淳渚淀淋淨猪菅菫菖萄菩萌萠菱陷悉敍晟晨晝脩梧梓梢梛梯桶梶椛梁條毬焔牽逞琢琉祷畢皐眸砦袴笹笙笠絃紬絆羚舵袈訣釧雀雫凱剩厨喬喧喰喋單堯堰堺寓巽惺捲揃搜渥湘湊湛葦葵萱葺萩董葡萬遥遁隈惹惣惡惠戟敦斐斯智曾脹腔棲椋椀欽盜焚煉爲犀瑛琥琶琵琳禄甥疏皓硯稀竣裡筈筑粥粟絢翔註貰釉舜閏雁傭傳嘩圓塙奧嵯嵩幌愼搖溢滉溜漣獅蓑蒔蒐蒼蒲蒙蓉蓮蒋蓬暉楯楚楕椿楠楓椰楢楊榊槌煌煤牒瑚瑞瑶禎碓碗碎祿禽稔稟稜裟裝詢詫跨靖頌馴馳鳩鼎僞嘉嘗噌團壽奬實寢嶋摺漱漕滯蔭蔦蔓斡暢膏榎樺榛槙槇槍樋榮熙爾遙瑳盡碩碧窪竪箕箔粹綺綜綴緋綾綸翠聡肇與蝋裳賑輔銑閤鞄頗颯魁鳶鳳齊價儉凜凛劉劍噂嬉幡廟廣彈憐撒撰撞播撫澁蕎蕨蕉蕃蕪遼鄭慧槻樟樂樣歎毅磐稻篇箪糊蝦蝶蝉誼諏諄諒賣醇醉鋒鞍駕駈髮魯鴎黎勳叡徽澪薙蕾蕗薗險戰曉樫橘樽橙燎燒燈燕窺瓢篠縞縣衞謂諺豬蹄輯醐醍錘鋸錐錆錫靜鞘鮎鴨黛默龍儲壕嶺彌擢濡濕藁薩應戲曙檜檎檀櫛檢燦燭瞥磯禪穗襖繋繍縱螺謠輿醤鍬霞鞠駿鴻叢壘藝藏藥櫂燿禰禮穣蹟轉鎧鎭雛雜鞭鯉鵜麿壞寵懷瀧蘇蘭曝櫓獸簾蟹顛鯛鵬麒嚴孃耀巌瀕纂饗馨騷攝灘櫻纏轟飜鰯鷄臟疊穰聽覽讃鑄驍巖纖顯驗鱒鱗鷲麟讓釀鷺鷹廳
''',
    u'''
几仄仇什曰叩伜卍吊吃牝佇呆吠囮妓屁巫彷扮肛杓杞牢禿乖侘卦咎呟呵呻姐姑妾怯拗沽泄狗狐狛炒爬軋剃叛咳咤垢姦姜屍屏峙徊恍拮洩洒狡茹茫扁胚枷柩歪炸砒袂虻酊酋俯冤唖埃埒屓悍捏涜涎涅浣狸狽拿胱脆烙逅疹疼疱眩罠祟舐蚤啖唸埠娼屠徘悴悸掻掟淘猜脛梱桿焉烹痔痒笥粕蛋蛆訛訝貶躯扈勒喘壺奢媚幇徨愕揶揄揉渾猥葱悶晰腋脾腑棍椒毯焙琲甦痙痣筍筏絨蛙蛭蛤覗詛跋靱韮黍嗜猾隕腿腱楡爺牌痰痺睨罫褌蛾蜃蛸裔詭誅跪辟酩雹飴鼠厭嘘嘔塵滲漉慇敲膀槃煽睾翡蝕誑銚儚噛嬌憔憮憫撓戮膣膵皺瞑磔磋褪蝿誹麹鋲鋤餃澱澹薔薇憑燐瞠瞞諜諫蹂踵躾錨閻餐頸頷髭髷鮒濤獰懃膿朦臀邂癌瞰糠糞縷謗賽鍾鍼鍍闊餞顆鮭鮫鮪鮨濾燻瞼謳謬贅贄魍魎檻蟻蠍贋躇轍騙鯵鯖鯱鯰鹸懺灌瀾朧礫躁饉饅鰐鰊囁爛贔躊饒魑鰤鶯聾贖轢驕鰻鱈攣讐躙鰹
''', ]


char_lines = [digit_line, symbol_line, punc_line, letter_line,
              hiragana_line, katakana_line, ]
char_lines.extend(kanji_lines)

chars = to_chars(char_lines)
digits = to_chars([digit_line])
symbols = to_chars([symbol_line])

ch_id_list = [(ch, i) for i, ch in enumerate(chars)]
ch_id_dict = {ch: i for i, ch in enumerate(chars)}


font_configs = [
    {
        'paths': (
            '/usr/share/fonts/truetype/fonts-japanese-gothic.ttf',
            '/home/csuncs89/0-priceless/programming/ocrus0_dataset/jp_fonts/antique-maru/font_1_ant-maru.ttf',
            '/home/csuncs89/0-priceless/programming/ocrus0_dataset/jp_fonts/NotoSansMonoCJKjp-Regular.otf',
            '/home/csuncs89/0-priceless/programming/ocrus0_dataset/jp_fonts/03SmartFontUI.ttf',
            '/home/csuncs89/0-priceless/programming/ocrus0_dataset/jp_fonts/07LogoTypeGothic7.ttf',
        ),
        'chars': chars,
        'sample': u'0 1 2 3 4 5 6 7 8 9 / ￥ 円 年 月 日 - / - ,  ， 。 \' "',
        'ratios': [1.1, 1.3]
    },
    {
        'paths': (
            '/home/csuncs89/0-priceless/programming/ocrus0_dataset/jp_fonts/Harenosora.otf',
        ),
        'chars': chars,
        'sample': u'0 1 2 3 4 5 6 7 8 9 / ￥ 円 年 月 日 - / - ,  ， 。 \' "',
        'ratios': [1.1, 1.3, 1.5, 1.7, 1.9]
    },
    {
        'paths': (
            '/home/csuncs89/0-priceless/programming/ocrus0_dataset/jp_fonts/RepetitionScrolling.ttf',
            '/media/sf_D_DRIVE/Morisawa_144/RyuminPro/G-OTF-GRyuminPro-Medium.otf',
            '/media/sf_D_DRIVE/Morisawa_144/ShinGoPro/G-OTF-GShinGoPro-Regular.otf',
            '/media/sf_D_DRIVE/Morisawa_144/Symbols/A-OTF-AntiqueStd-AN3.otf',
        ),
        'chars': digits,
        'sample': u'0 1 2 3 4 5 6 7 8 9',
        'ratios': [1.1, 1.3]
    },

    {'paths': ('/media/sf_D_DRIVE/fonts/Fonts_win7_jp/mingliu.ttc',
               '/media/sf_D_DRIVE/fonts/Fonts_win7_jp/gulim.ttc',),
     'chars': symbols,
     'sample': u'￥',
     'ratios': [1.1, 1.3, 1.5, 1.7, 1.9]
     }
]


im = Image.new("L", (2000, 2000), color=255)
draw = ImageDraw.Draw(im)
font_i = 0
for font_config in font_configs:
    for font_path in font_config['paths']:
        font = ImageFont.truetype(font_path, 80, encoding='utf-8')
        for text_i, ch in enumerate(to_chars([font_config['sample']])):
            draw.text(
                (80 * text_i, 120 * font_i), ch, font=font, fill="#000000")
        font_i += 1
im.save('/home/csuncs89/0-priceless/programming/ocrus0_build/sample_text.png')

with io.open(PATH_CHARS_ID, 'w', encoding='utf-8') as json_file:
    data = json.dumps(ch_id_list, ensure_ascii=False, indent=2)
    json_file.write(unicode(data))

print 'There are', len(ch_id_list), 'chars'

training_data = ([], [])
validation_data = ([], [])
test_data = ([], [])

CANVAS_W = 100
CANVAS_H = 100
FONT_SIZE = 60

for font_config in font_configs:
    for font_path in font_config['paths']:
        print
        print font_path

        font = ImageFont.truetype(font_path, FONT_SIZE, encoding='utf-8')

        font_chars = font_config['chars']
        if SHOW_CV2_CHAR:
            font_chars = FONT_CHARS_CV2

        for ch in font_chars:
            char_id = ch_id_dict[ch]
            if char_id % 10 == 0:
                print char_id,
                sys.stdout.flush()

            im = Image.new("L", (CANVAS_W, CANVAS_H), color=255)
            draw = ImageDraw.Draw(im)
            draw.text((20, 0), ch, font=font, fill="#000000")
            # im.save('img.png')

            im_array = np.array(im.getdata(), np.uint8)
            im_array = im_array.reshape(CANVAS_H, CANVAS_W)

            sizes = [(CANVAS_W, CANVAS_H)]
            for ratio in font_config['ratios']:
                sizes.append(
                    (CANVAS_W, int(CANVAS_H * (ratio + random.random() / 20))))
                sizes.append(
                    (int(CANVAS_W * (ratio + random.random() / 20)), CANVAS_H))

            float_imgs = []
            for sz in sizes:
                # print (im_array.shape[1], im_array.shape[0]), '->', sz
                resized_img = cv2.resize(im_array, sz)
                float_img = gray_img_to_mnist_array(
                    remove_white_border(resized_img))
                float_imgs.append(float_img)

                if SHOW_CV2_CHAR:
                    cv2.namedWindow("img", cv2.WINDOW_NORMAL)
                    cv2.imshow("img", mnist_array_to_gray_img(float_img))
                    cv2.waitKey()

            for float_img in float_imgs:
                if SHOW_CHAR:
                    pretty_print_char(float_img)
                    print

                r = random.randint(0, 9)
                if r in [0, 1, 2, 3, 4, 5, 6, ]:
                    training_data[0].append(float_img)
                    training_data[1].append(char_id)
                elif r in [7]:
                    validation_data[0].append(float_img)
                    validation_data[1].append(char_id)
                else:
                    test_data[0].append(float_img)
                    test_data[1].append(char_id)

                if SHOW_CHAR:
                    time.sleep(SHOW_SLEEP)

print 'There are', len(ch_id_list), 'chars'

training_data = map(np.asarray, (training_data[0], training_data[1]))
validation_data = map(np.asarray, (validation_data[0], validation_data[1]))
test_data = map(np.asarray, (test_data[0], test_data[1]))

print map(len, (training_data[0], validation_data[0], test_data[0]))

f = gzip.open(PATH_PKL_GZ, 'wb')
cPickle.dump((training_data, validation_data, test_data), f)
