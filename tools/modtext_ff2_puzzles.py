"""Original instructions plus exact official FF2 terms in every supported language.

Read only selected system-message terms from one native bundle, serially under
the resource runner. Never export dialogue or native implementation into Git.
"""
import hashlib,json,re,sys
from pathlib import Path
from modtext_opera_guide import LOCALES
ROOT=Path(__file__).resolve().parents[1]
P=ROOT/'data/common/mod_text.json'
ROWS={
'Speak to the boy.':('Speak to the boy.','少年に話しかけてください。','Parlez au garçon.','Parla con il ragazzo.','Sprich mit dem Jungen.','Habla con el niño.','소년에게 말을 거세요.','與男孩交談。','与男孩交谈。','Поговорите с мальчиком.','Fale com o menino.','พูดคุยกับเด็กชาย'),
'Follow the boy and speak to him again.':('Follow the boy and speak to him again.','少年を追い、もう一度話しかけてください。','Suivez le garçon et parlez-lui à nouveau.','Segui il ragazzo e parlagli di nuovo.','Folge dem Jungen und sprich ihn erneut an.','Sigue al niño y vuelve a hablar con él.','소년을 따라가 다시 말을 거세요.','跟隨男孩並再次與他交談。','跟随男孩并再次与他交谈。','Следуйте за мальчиком и снова поговорите с ним.','Siga o menino e fale com ele novamente.','ตามเด็กชายไปแล้วพูดคุยกับเขาอีกครั้ง'),
'Speak to the mother and learn {0}. Repeat the guide after learning it.':('Speak to the mother and learn {0}. Repeat the guide after learning it.','母親に話しかけ、{0}を覚えてください。覚えたらガイドを繰り返してください。','Parlez à la mère et apprenez {0}. Répétez le guide après cet apprentissage.','Parla con la madre e impara {0}. Ripeti la guida dopo averlo imparato.','Sprich mit der Mutter und lerne {0}. Wiederhole danach die Anleitung.','Habla con la madre y aprende {0}. Repite la guía después de aprenderlo.','어머니에게 말을 걸어 {0}을 배우세요. 배운 후 안내를 반복하세요.','與母親交談並記住{0}。記住後重複指南。','与母亲交谈并记住{0}。记住后重复指南。','Поговорите с матерью и выучите {0}. Затем повторите подсказку.','Fale com a mãe e aprenda {0}. Repita o guia após aprender.','พูดคุยกับแม่แล้วเรียนรู้ {0} จากนั้นเรียกคำแนะนำซ้ำ'),
'Examine the pendant.':('Examine the pendant.','ペンダントを調べてください。','Examinez le pendentif.','Esamina il pendente.','Untersuche den Anhänger.','Examina el colgante.','펜던트를 조사하세요.','調查墜飾。','调查坠饰。','Осмотрите кулон.','Examine o pingente.','สำรวจจี้'),
'Ask the wyvern about {0} to receive its egg.':('Ask the wyvern about {0} to receive its egg.','飛竜に{0}について尋ね、卵を受け取ってください。',"Interrogez la wyverne sur {0} pour recevoir son œuf.",'Chiedi alla viverna di {0} per ricevere il suo uovo.','Frage den Wyvern nach {0}, um sein Ei zu erhalten.','Pregunta al wyvern por {0} para recibir su huevo.','와이번에게 {0}에 대해 물어 알을 받으세요.','向飛龍詢問{0}以取得牠的蛋。','向飞龙询问{0}以取得它的蛋。','Спросите виверну о {0}, чтобы получить яйцо.','Pergunte à serpe sobre {0} para receber seu ovo.','ถามไวเวิร์นเกี่ยวกับ {0} เพื่อรับไข่'),
'Follow the safe stairs past the B3 pitfall, then defeat the guardian.':('Follow the safe stairs past the B3 pitfall, then defeat the guardian.','地下3階の落とし穴を避けて安全な階段を進み、守護者を倒してください。','Suivez les escaliers sûrs en évitant la fosse du troisième sous-sol, puis battez le gardien.','Segui le scale sicure evitando la trappola al terzo sotterraneo, poi sconfiggi il guardiano.','Nimm die sicheren Treppen an der Fallgrube im dritten Untergeschoss vorbei und besiege den Wächter.','Sigue las escaleras seguras evitando la trampa del tercer sótano y derrota al guardián.','지하 3층의 함정을 피해 안전한 계단을 따라간 다음 수호자를 쓰러뜨리세요.','沿安全樓梯避開地下3樓的陷阱，然後擊敗守護者。','沿安全楼梯避开地下3楼的陷阱，然后击败守护者。','Идите по безопасным лестницам в обход ямы на третьем подземном этаже, затем победите стража.','Siga pelas escadas seguras evitando o alçapão do terceiro subsolo e derrote o guardião.','ใช้บันไดที่ปลอดภัยหลบหลุมกับดักชั้นใต้ดิน 3 แล้วเอาชนะผู้พิทักษ์'),
'Use {0} at the Life Spring.':('Use {0} at the Life Spring.','命の泉で{0}を使ってください。','Utilisez {0} à la source de vie.','Usa {0} alla Fonte della Vita.','Benutze {0} an der Lebensquelle.','Usa {0} en el Manantial de la Vida.','생명의 샘에서 {0}을 사용하세요.','在生命之泉使用{0}。','在生命之泉使用{0}。','Используйте {0} у Источника жизни.','Use {0} na Fonte da Vida.','ใช้ {0} ที่น้ำพุแห่งชีวิต'),
}
from modtext_ff2_gates import ROWS as GATE_ROWS
ROWS.update(GATE_ROWS)
TERMS={'FF2 keyword Wyverns':'MSG_IMP_WORD_11','FF2 item Wyvern Egg':'MSG_KEY_NAME_10',
 'FF2 keyword Mysidia':'MSG_IMP_WORD_08','FF2 keyword Mask':'MSG_IMP_WORD_14','FF2 keyword Ekmet Teloess':'MSG_IMP_WORD_15',
 'FF2 keyword Wild Rose':'MSG_IMP_WORD_01','FF2 item Goddess Bell':'MSG_KEY_NAME_06','FF2 item Egil Torch':'MSG_KEY_NAME_07',
 'FF2 item Sunfire':'MSG_KEY_NAME_08','FF2 item White Mask':'MSG_KEY_NAME_11','FF2 item Black Mask':'MSG_KEY_NAME_12','FF2 item Pass':'MSG_KEY_NAME_03',
 'FF2 keyword Cyclone':'MSG_IMP_WORD_12','FF2 item Pendant':'MSG_KEY_NAME_09'}
ASSET_LOCALES={locale:locale for locale in LOCALES}

def main():
    sys.path.insert(0,str(ROOT.parent/'tools'));import dump_map_objects as archive
    bundles=list(Path(archive.aa_path('ff2')).glob('message_assets_*.bundle'));assert len(bundles)==1
    _,assets=archive.load_texts(str(bundles[0]));rows={key:dict(zip(LOCALES,values)) for key,values in ROWS.items()}
    assert all(len(values)==12 and values[0]==key and all(values) for key,values in ROWS.items())
    for key,message in TERMS.items():
        localized={}
        for locale in LOCALES:
            texts=assets['system_'+ASSET_LOCALES[locale]];assert len(texts)==1
            terms=dict(line.rstrip('\r\n').split('\t',1) for line in texts[0].splitlines() if '\t' in line)
            localized[locale]=re.sub(r'<[^>]*>','',terms[message]).strip();assert localized[locale] and '<' not in localized[locale]
        rows[key]=localized
    original=P.read_text(encoding='utf-8');catalog=json.loads(original)
    # Official item entries include a native icon token. Update only our known
    # term rows when normalizing it, preserving every unrelated catalog byte.
    for key in TERMS:
        if key in catalog and catalog[key]!=rows[key]:
            if '--check' in sys.argv:raise AssertionError(('FF2 official term differs',key))
            start=original.index(json.dumps(key,ensure_ascii=False)+': ')
            start=original.index('{',start);old,end=json.JSONDecoder().raw_decode(original[start:]);assert old==catalog[key]
            replacement=json.dumps(rows[key],ensure_ascii=False,indent=2).replace('\n','\n  ')
            original=original[:start]+replacement+original[start+end:];catalog[key]=rows[key]
    if '--check' not in sys.argv and original!=P.read_text(encoding='utf-8'):P.write_text(original,encoding='utf-8')
    additions={key:value for key,value in rows.items() if key not in catalog}
    if '--check' in sys.argv:assert not additions,'Missing FF2 puzzle instructions/terms'
    if additions:
        insertion=json.dumps(additions,ensure_ascii=False,indent=2)[2:-2]
        P.write_text(original[:original.rfind('}')].rstrip()+',\n'+insertion+'\n}\n',encoding='utf-8');catalog.update(additions)
    for key,value in rows.items():assert catalog[key]==value,('FF2 puzzle translation differs',key)
    report=dict(instructions=len(ROWS),official_terms=len(TERMS),locales=LOCALES,bundle=bundles[0].name,
        bundle_sha256=hashlib.sha256(bundles[0].read_bytes()).hexdigest(),terms=TERMS)
    (ROOT/'build_check/ff2_puzzle_locales.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print('FF2 puzzles:',len(ROWS),'instructions and',len(TERMS),'exact official terms in twelve languages;',len(additions),'added')

if __name__=='__main__':main()
