"""Six original constrained-party instructions in every supported language."""
from pathlib import Path
import json,sys
from author_ff5_fork import PREP,MAGIC,PHYSICAL,RIGHT,LEFT
KEYS=(PREP,MAGIC,PHYSICAL,RIGHT,LEFT,'Use magic in the left tower.')
TEXT={
'en':KEYS,
'ja':('左の塔には魔法を使う者、右の塔には物理攻撃をする者を用意してください。','渡る前に、このキャラクターが魔法を使えるよう準備してください。','渡る前に、このキャラクターの物理攻撃を準備してください。','物理攻撃で勝ってください。魔法は使わないでください。','すぐに魔法を取り、魔法を使って戦闘に勝ってください。','左の塔では魔法を使ってください。'),
'fr':('Préparez des mages pour la tour gauche et des combattants physiques pour la tour droite.','Avant de traverser, préparez ce personnage à utiliser la magie.','Avant de traverser, préparez ce personnage aux attaques physiques.','Gagnez ce combat avec des attaques physiques. N’utilisez pas de magie.','Prenez immédiatement le sort, puis gagnez le combat avec la magie.','Utilisez la magie dans la tour gauche.'),
'it':('Prepara maghi per la torre sinistra e attaccanti fisici per la torre destra.','Prima di attraversare, prepara questo personaggio a usare la magia.','Prima di attraversare, prepara questo personaggio agli attacchi fisici.','Vinci con attacchi fisici. Non usare magia.','Prendi subito la magia, poi vinci lo scontro usando la magia.','Usa la magia nella torre sinistra.'),
'de':('Bereite Magier für den linken Turm und physische Angreifer für den rechten Turm vor.','Bereite diesen Charakter vor dem Überqueren auf Magie vor.','Bereite diesen Charakter vor dem Überqueren auf physische Angriffe vor.','Gewinne diesen Kampf mit physischen Angriffen. Benutze keine Magie.','Nimm sofort den Zauber und gewinne den Kampf mit Magie.','Benutze im linken Turm Magie.'),
'es':('Prepara usuarios de magia para la torre izquierda y atacantes físicos para la derecha.','Antes de cruzar, prepara a este personaje para usar magia.','Antes de cruzar, prepara a este personaje para ataques físicos.','Gana este combate con ataques físicos. No uses magia.','Toma el hechizo de inmediato y gana el combate usando magia.','Usa magia en la torre izquierda.'),
'ko':('왼쪽 탑에는 마법 사용자, 오른쪽 탑에는 물리 공격자를 준비하세요.','건너기 전에 이 캐릭터가 마법을 쓸 수 있도록 준비하세요.','건너기 전에 이 캐릭터의 물리 공격을 준비하세요.','물리 공격으로 이 전투에서 승리하세요. 마법을 쓰지 마세요.','즉시 마법을 얻은 뒤 마법으로 전투에서 승리하세요.','왼쪽 탑에서는 마법을 쓰세요.'),
'zht':('為左塔準備魔法使用者，為右塔準備物理攻擊者。','通過之前，讓此角色準備好使用魔法。','通過之前，讓此角色準備好進行物理攻擊。','使用物理攻擊贏得這場戰鬥。不要使用魔法。','立即取得魔法，然後用魔法贏得戰鬥。','在左塔使用魔法。'),
'zhc':('为左塔准备魔法使用者，为右塔准备物理攻击者。','通过之前，让此角色准备好使用魔法。','通过之前，让此角色准备好进行物理攻击。','使用物理攻击赢得这场战斗。不要使用魔法。','立即取得魔法，然后用魔法赢得战斗。','在左塔使用魔法。'),
'ru':('Подготовьте магов для левой башни и бойцов с физическими атаками для правой.','Перед переходом подготовьте этого персонажа к использованию магии.','Перед переходом подготовьте этого персонажа к физическим атакам.','Победите физическими атаками. Не используйте магию.','Немедленно возьмите заклинание, затем победите с помощью магии.','Используйте магию в левой башне.'),
'pt':('Prepare usuários de magia para a torre esquerda e atacantes físicos para a direita.','Antes de atravessar, prepare este personagem para usar magia.','Antes de atravessar, prepare este personagem para ataques físicos.','Vença esta batalha com ataques físicos. Não use magia.','Pegue a magia imediatamente e vença a batalha usando magia.','Use magia na torre esquerda.'),
'th':('เตรียมผู้ใช้เวทมนตร์ไว้สำหรับหอคอยซ้าย และผู้โจมตีกายภาพไว้สำหรับหอคอยขวา','ก่อนข้าม ให้เตรียมตัวละครนี้ให้ใช้เวทมนตร์ได้','ก่อนข้าม ให้เตรียมตัวละครนี้สำหรับการโจมตีกายภาพ','ชนะการต่อสู้นี้ด้วยการโจมตีกายภาพ อย่าใช้เวทมนตร์','รับเวทมนตร์ทันที แล้วชนะการต่อสู้ด้วยเวทมนตร์','ใช้เวทมนตร์ในหอคอยซ้าย'),
}
def main():
 check='--check'in sys.argv;p=Path(__file__).resolve().parents[1]/'data/ff5/mod_text.json';d=json.loads(p.read_text(encoding='utf-8'));assert len(TEXT)==12 and all(len(v)==6 for v in TEXT.values())
 for i,key in enumerate(KEYS):
  wanted={lang:v[i]for lang,v in TEXT.items()}
  if check:assert d.get(key)==wanted,key
  else:assert key not in d or d[key]==wanted;d[key]=wanted
 if not check:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Fork:six original instructions/all12 supported locales.')
if __name__=='__main__':main()
