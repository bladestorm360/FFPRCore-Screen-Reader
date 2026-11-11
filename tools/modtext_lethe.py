"""Original short raft instructions,official Banon name in all12 locales."""
from pathlib import Path
import hashlib,json,sys
from author_lethe import BOARD,LOOP,BOSS
R=Path(__file__).resolve().parents[1]
TEXT={
'en':(BOARD,LOOP,'Board the raft, then win the battle. Keep {name} alive.'),
'ja':('いかだに乗り、分岐では最初の選択肢を選んでください。','いかだに乗り、次の分岐では2番目の選択肢を選んでください。','いかだに乗り、戦闘に勝ってください。{name}を倒されないように守ってください。'),
'fr':('Montez sur le radeau. À la bifurcation, choisissez la première option.','Montez sur le radeau. À la prochaine bifurcation, choisissez la deuxième option.','Montez sur le radeau, puis gagnez le combat. Gardez {name} en vie.'),
'it':('Sali sulla zattera. Al bivio scegli la prima opzione.','Sali sulla zattera. Al prossimo bivio scegli la seconda opzione.','Sali sulla zattera e vinci la battaglia. Tieni in vita {name}.'),
'de':('Steige auf das Floß. Wähle an der Abzweigung die erste Option.','Steige auf das Floß. Wähle an der nächsten Abzweigung die zweite Option.','Steige auf das Floß und gewinne den Kampf. Halte {name} am Leben.'),
'es':('Sube a la balsa. En la bifurcación, elige la primera opción.','Sube a la balsa. En la siguiente bifurcación, elige la segunda opción.','Sube a la balsa y gana la batalla. Mantén con vida a {name}.'),
'ko':('뗏목에 타세요. 갈림길에서 첫 번째 선택지를 고르세요.','뗏목에 타세요. 다음 갈림길에서는 두 번째 선택지를 고르세요.','뗏목에 타고 전투에서 승리하세요. {name}을 생존시키세요.'),
'zht':('搭上木筏。在岔路選擇第一個選項。','搭上木筏。在下一個岔路選擇第二個選項。','搭上木筏並贏得戰鬥。保護{name}存活。'),
'zhc':('搭上木筏。在岔路选择第一个选项。','搭上木筏。在下一个岔路选择第二个选项。','搭上木筏并赢得战斗。保护{name}存活。'),
'ru':('Садитесь на плот. На развилке выберите первый вариант.','Садитесь на плот. На следующей развилке выберите второй вариант.','Садитесь на плот и выиграйте бой. Сохраните жизнь персонажу {name}.'),
'th':('ขึ้นแพแล้วเลือกตัวเลือกแรกเมื่อถึงทางแยก','ขึ้นแพแล้วเลือกตัวเลือกที่สองเมื่อถึงทางแยกถัดไป','ขึ้นแพแล้วชนะการต่อสู้ รักษา {name} ให้มีชีวิตอยู่'),
'pt':('Embarque na jangada. Na bifurcação, escolha a primeira opção.','Embarque na jangada. Na próxima bifurcação, escolha a segunda opção.','Embarque na jangada e vença a batalha. Mantenha {name} vivo.'),
}
def expected():
 out={k:{}for k in(BOARD,LOOP,BOSS)}
 for lang,texts in TEXT.items():
  p=R.parent/'tools/mapdump/ff6_master'/('system_'+lang+'.txt');names={}
  for line in p.read_text(encoding='utf-8-sig').splitlines():
   if '\t'in line:k,v=line.split('\t',1);names[k]=v
  name=names['MSG_CHAR_NAME_15'];assert name and len(name)<64
  for key,value in zip(out,texts):out[key][lang]=value.format(name=name)
 assert out[BOSS]['en']==BOSS
 return out
if __name__=='__main__':
 p=R/'data/ff6/mod_text.json';data=json.loads(p.read_text(encoding='utf-8'));want=expected()
 if '--check'in sys.argv:assert all(data.get(k)==v for k,v in want.items())
 else:data.update(want);p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Lethe:three short instructions/all12 languages,official character names;existing ordinal answers reused.')
