"""Concise choice and optional-mechanic guidance in all twelve game locales."""
from pathlib import Path
import json,sys
from author_cyan_zozo import READ,BUY,LETTER,DRAGON
ROOT=Path(__file__).resolve().parents[1]
TEXT={
'en':(READ,BUY,LETTER,DRAGON),
'ja':('最初の選択肢で手紙を読み、次も最初の選択肢で返事の配達を引き受けてください。','最初の選択肢を選び、1000ギルでサビトレールを買ってください。','マランダでローラの返事を伝書鳩に渡し、ゾゾに戻ってください。','任意：このスイッチでドラゴンを放ち、戦闘に勝ってください。'),
'fr':('Choisissez la première option pour lire la lettre, puis la première pour livrer la réponse.','Choisissez la première option pour acheter le dérouillant pour 1000 gils.','Remettez la réponse de Lola au pigeon à Maranda, puis revenez à Zozo.','Facultatif : actionnez cet interrupteur pour libérer le dragon, puis gagnez le combat.'),
'it':('Scegli la prima opzione per leggere la lettera, poi la prima per consegnare la risposta.','Scegli la prima opzione per comprare l’antiruggine per 1000 guil.','Consegna la risposta di Lola al piccione a Maranda, poi torna a Zozo.','Facoltativo: aziona questo interruttore per liberare il drago, poi vinci la battaglia.'),
'de':('Wähle die erste Option, um den Brief zu lesen, und danach die erste, um die Antwort zu überbringen.','Wähle die erste Option, um den Rostlöser für 1000 Gil zu kaufen.','Gib Lolas Antwort der Taube in Maranda und kehre dann nach Zozo zurück.','Optional: Betätige diesen Schalter, um den Drachen freizulassen, und gewinne dann den Kampf.'),
'es':('Elige la primera opción para leer la carta y luego la primera para entregar la respuesta.','Elige la primera opción para comprar el antioxidante por 1000 guiles.','Entrega la respuesta de Lola a la paloma en Maranda y vuelve a Zozo.','Opcional: activa este interruptor para liberar al dragón y gana la batalla.'),
'ko':('첫 번째 선택지로 편지를 읽고, 다음에도 첫 번째 선택지로 답장 배달을 맡으세요.','첫 번째 선택지로 녹 제거제를 1000길에 사세요.','마란다에서 로라의 답장을 비둘기에게 전한 뒤 조조로 돌아오세요.','선택 사항: 이 스위치로 드래곤을 풀어 준 뒤 전투에서 승리하세요.'),
'zht':('選第一個選項閱讀信件，再選第一個選項接受送回信。','選第一個選項，以1000金幣購買除鏽劑。','在瑪蘭達把蘿拉的回信交給信鴿，再回到佐佐。','非必要：操作此開關釋放巨龍，再贏得戰鬥。'),
'zhc':('选第一个选项阅读信件，再选第一个选项接受送回信。','选第一个选项，以1000金币购买除锈剂。','在玛兰达把萝拉的回信交给信鸽，再回到佐佐。','非必要：操作此开关释放巨龙，再赢得战斗。'),
'ru':('Выберите первый вариант, чтобы прочитать письмо, затем первый, чтобы доставить ответ.','Выберите первый вариант, чтобы купить средство от ржавчины за 1000 гил.','Передайте ответ Лолы голубю в Маранде, затем вернитесь в Зозо.','По желанию: нажмите этот переключатель, чтобы выпустить дракона, затем победите в бою.'),
'pt':('Escolha a primeira opção para ler a carta e depois a primeira para entregar a resposta.','Escolha a primeira opção para comprar o removedor de ferrugem por 1000 gil.','Entregue a resposta de Lola ao pombo em Maranda e volte a Zozo.','Opcional: acione este interruptor para soltar o dragão e vença a batalha.'),
'th':('เลือกตัวเลือกแรกเพื่ออ่านจดหมาย แล้วเลือกตัวเลือกแรกเพื่อรับส่งจดหมายตอบกลับ','เลือกตัวเลือกแรกเพื่อซื้อน้ำยาขจัดสนิมในราคา 1000 กิล','ส่งจดหมายตอบกลับของโลลาให้นกพิราบที่มารันดา แล้วกลับไปโซโซ','ทำหรือไม่ก็ได้: ใช้สวิตช์นี้เพื่อปล่อยมังกร แล้วชนะการต่อสู้'),
}
if __name__=='__main__':
 assert len(TEXT)==12
 p=ROOT/'data/ff6/mod_text.json';data=json.loads(p.read_text(encoding='utf-8'))
 for index,key in enumerate(TEXT['en']):
  want={lang:values[index]for lang,values in TEXT.items()};assert all(want.values())
  if '--check'in sys.argv:assert data.get(key)==want
  else:assert key not in data or data[key]==want;data[key]=want
 if '--check'not in sys.argv:p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Cyan/Zozo:four instructions/all12 locales;original independent navigation/choice policy retained.')
