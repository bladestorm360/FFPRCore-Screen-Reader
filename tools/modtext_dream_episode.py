"""Three original Dreamscape instructions in all supported mod languages."""
from pathlib import Path
import json,sys
from author_dream_episode import PARTY,REST,POSSESSION
ROOT=Path(__file__).resolve().parents[1]
TEXT={
'en':(PARTY,REST,POSSESSION),
'ja':('カイエンを含む４人のパーティーで来てください。','北へ１歩進んでベッドに入り、最初の選択肢を選んで休んでください。','ボスと戦ってください。消えたら、味方を１人ずつ戦闘不能にして蘇生し、ボスが戻るまで繰り返してください。必ず１人は生存させてください。戻ったボスを攻撃し、必要なら繰り返してください。'),
'fr':('Venez avec Cyan dans une équipe active complète de quatre personnages.','Faites un pas au nord dans le lit, puis choisissez la première option pour vous reposer.','Combattez le boss. Quand il disparaît, mettez les alliés K.O. puis ranimez-les un à un jusqu’à son retour. Gardez toujours un allié en vie. Attaquez le boss à son retour et répétez si nécessaire.'),
'it':('Porta Cyan in un gruppo attivo completo di quattro personaggi.','Fai un passo a nord nel letto, poi scegli la prima opzione per riposare.','Affronta il boss. Quando scompare, manda K.O. e rianima gli alleati uno alla volta finché ritorna. Tieni sempre un alleato in vita. Attacca il boss quando ritorna e ripeti se necessario.'),
'de':('Komm mit Cyan in einer aktiven Gruppe aus vier Charakteren.','Gehe einen Schritt nach Norden ins Bett und wähle dann die erste Option zum Ausruhen.','Bekämpfe den Boss. Wenn er verschwindet, schlage Verbündete einzeln K.O. und belebe sie wieder, bis er zurückkehrt. Halte einen Verbündeten am Leben. Greife den Boss bei seiner Rückkehr an und wiederhole dies bei Bedarf.'),
'es':('Lleva a Cyan en un grupo activo completo de cuatro personajes.','Da un paso al norte hacia la cama y elige la primera opción para descansar.','Lucha contra el jefe. Cuando desaparezca, deja fuera de combate y revive a los aliados de uno en uno hasta que vuelva. Mantén vivo a un aliado. Ataca al jefe cuando vuelva y repite si hace falta.'),
'ko':('카이엔을 포함한 4명의 전투 파티로 오세요.','북쪽으로 한 걸음 이동해 침대에 들어간 뒤 첫 번째 선택지를 골라 쉬세요.','보스와 싸우세요. 보스가 사라지면 돌아올 때까지 아군을 한 명씩 전투 불능으로 만들고 부활시키세요. 반드시 한 명은 살아 있게 하세요. 보스가 돌아오면 공격하고 필요하면 반복하세요.'),
'zht':('請讓凱嚴加入四人的出戰隊伍。','向北走一步進入床位，然後選擇第一個選項休息。','與首領戰鬥。當它消失時，逐一讓隊友無法戰鬥再復活，直到它返回。務必保留一名存活隊友。首領返回後攻擊它，必要時重複。'),
'zhc':('请让凯严加入四人的出战队伍。','向北走一步进入床位，然后选择第一个选项休息。','与首领战斗。当它消失时，逐一让队友无法战斗再复活，直到它返回。务必保留一名存活队友。首领返回后攻击它，必要时重复。'),
'ru':('Возьмите Циана в полную действующую группу из четырёх персонажей.','Сделайте шаг на север к кровати и выберите первый вариант, чтобы отдохнуть.','Сразитесь с боссом. Когда он исчезнет, по очереди отправляйте союзников в нокаут и воскрешайте их, пока он не вернётся. Оставляйте одного союзника в живых. Атакуйте босса после возвращения и повторяйте при необходимости.'),
'pt':('Traga Cyan em um grupo ativo completo de quatro personagens.','Dê um passo ao norte para a cama e escolha a primeira opção para descansar.','Enfrente o chefe. Quando ele sumir, nocauteie e reviva os aliados, um de cada vez, até ele voltar. Mantenha um aliado vivo. Ataque o chefe quando voltar e repita se necessário.'),
'th':('ให้ไคเอ็นอยู่ในกลุ่มต่อสู้ที่มีสมาชิกครบสี่คน','เดินขึ้นเหนือหนึ่งก้าวเข้าที่นอน แล้วเลือกตัวเลือกแรกเพื่อพักผ่อน','ต่อสู้กับบอส เมื่อบอสหายไป ให้ทำให้เพื่อนหมดสติและชุบชีวิตทีละคนจนกว่าบอสจะกลับมา ต้องให้เพื่อนมีชีวิตอยู่อย่างน้อยหนึ่งคน โจมตีบอสเมื่อกลับมาและทำซ้ำหากจำเป็น'),
}
if __name__=='__main__':
 assert len(TEXT)==12
 p=ROOT/'data/ff6/mod_text.json';data=json.loads(p.read_text(encoding='utf-8'))
 for index,key in enumerate(TEXT['en']):
  want={lang:values[index]for lang,values in TEXT.items()};assert all(want.values())
  if '--check'in sys.argv:assert data.get(key)==want
  else:assert key not in data or data[key]==want;data[key]=want
 if '--check'not in sys.argv:p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Dream:three instructions/all12 languages;active-party setup,south bed approach and possession solution.')
