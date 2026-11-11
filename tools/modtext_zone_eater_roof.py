"""Twelve-language roof-guide controls; preserve every unrelated catalog key."""
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
LANGS=('en','ja','fr','it','de','es','ko','zht','zhc','ru','pt','th')
ROWS=[
('Go now.',
 '今、進んでください。','Allez-y maintenant.','Vai ora.','Jetzt los.','Avanza ahora.','지금 이동하세요.','現在走。','现在走。','Идите сейчас.','Vá agora.','ไปตอนนี้'),
('Wait here for Go now.',
 'ここで「今、進んでください」の合図を待ってください。','Attendez ici le signal « Allez-y maintenant ».','Aspetta qui il segnale « Vai ora ».','Warte hier auf das Signal „Jetzt los“.','Espera aquí la señal «Avanza ahora».','여기서 “지금 이동하세요”라는 신호를 기다리세요.','在這裡等待「現在走」的指示。','在这里等待“现在走”的指示。','Ждите здесь сигнала «Идите сейчас».','Espere aqui pelo sinal «Vá agora».','รออยู่ที่นี่จนกว่าจะได้ยินว่า “ไปตอนนี้”'),
('Next safe area: {0}. Request the guide again when ready, then wait for Go now.',
 '次の安全地帯：{0}。準備ができたらもう一度ガイドを呼び出し、「今、進んでください」の合図を待ってください。',
 'Prochaine zone sûre : {0}. Redemandez le guide quand vous êtes prêt, puis attendez « Allez-y maintenant ».','Prossima zona sicura: {0}. Richiedi di nuovo la guida quando sei pronto, poi aspetta « Vai ora ».','Nächster sicherer Bereich: {0}. Rufe die Anleitung erneut auf, wenn du bereit bist, und warte dann auf „Jetzt los“.','Siguiente zona segura: {0}. Solicita la guía de nuevo cuando estés listo y espera la señal «Avanza ahora».','다음 안전 지대: {0}. 준비되면 가이드를 다시 요청한 뒤 “지금 이동하세요”라는 신호를 기다리세요.','下一個安全區域：{0}。準備好後再次要求指引，然後等待「現在走」的指示。','下一个安全区域：{0}。准备好后再次请求指引，然后等待“现在走”的指示。','Следующее безопасное место: {0}. Когда будете готовы, снова запросите подсказку и ждите сигнала «Идите сейчас».','Próxima área segura: {0}. Peça o guia novamente quando estiver pronto e espere pelo sinal «Vá agora».','พื้นที่ปลอดภัยถัดไป: {0} เมื่อพร้อมแล้ว ให้ขอคำแนะนำอีกครั้ง จากนั้นรอจนกว่าจะได้ยินว่า “ไปตอนนี้”'),
('Your position changed. Request the roof guide again.',
 '位置が変わりました。天井のガイドをもう一度呼び出してください。','Votre position a changé. Redemandez le guide du plafond.','La tua posizione è cambiata. Richiedi di nuovo la guida del soffitto.','Deine Position hat sich geändert. Rufe die Deckenanleitung erneut auf.','Tu posición ha cambiado. Solicita de nuevo la guía del techo.','위치가 바뀌었습니다. 천장 가이드를 다시 요청하세요.','你的位置已變更。請再次要求天花板機關指引。','你的位置已改变。请再次请求天花板机关指引。','Ваше положение изменилось. Снова запросите подсказку по потолку.','A sua posição mudou. Peça o guia do teto novamente.','ตำแหน่งของคุณเปลี่ยนไปแล้ว โปรดขอคำแนะนำสำหรับเพดานอีกครั้ง'),
('Roof timing is not ready. Wait in a safe area, then request the guide again.',
 '天井のタイミングをまだ確認できません。安全地帯で待ち、ガイドをもう一度呼び出してください。','Le rythme du plafond n’est pas encore disponible. Attendez dans une zone sûre, puis redemandez le guide.','Il ritmo del soffitto non è ancora disponibile. Aspetta in una zona sicura, poi richiedi di nuovo la guida.','Der Deckenrhythmus ist noch nicht verfügbar. Warte in einem sicheren Bereich und rufe die Anleitung erneut auf.','El ritmo del techo aún no está disponible. Espera en una zona segura y vuelve a solicitar la guía.','천장 타이밍을 아직 확인할 수 없습니다. 안전 지대에서 기다린 뒤 가이드를 다시 요청하세요.','尚無法確認天花板的時機。請在安全區域等待，然後再次要求指引。','尚无法确认天花板的时机。请在安全区域等待，然后再次请求指引。','Ритм потолка пока неизвестен. Подождите в безопасном месте и снова запросите подсказку.','O ritmo do teto ainda não está disponível. Espere numa área segura e peça o guia novamente.','ยังตรวจสอบจังหวะเพดานไม่ได้ ให้รอในพื้นที่ปลอดภัย แล้วขอคำแนะนำอีกครั้ง'),
('Stop in the next safe area, then request the guide again.',
 '次の安全地帯で止まり、ガイドをもう一度呼び出してください。','Arrêtez-vous dans la prochaine zone sûre, puis redemandez le guide.','Fermati nella prossima zona sicura, poi richiedi di nuovo la guida.','Halte im nächsten sicheren Bereich an und rufe die Anleitung erneut auf.','Detente en la siguiente zona segura y vuelve a solicitar la guía.','다음 안전 지대에서 멈춘 뒤 가이드를 다시 요청하세요.','在下一個安全區域停下，然後再次要求指引。','在下一个安全区域停下，然后再次请求指引。','Остановитесь в следующем безопасном месте и снова запросите подсказку.','Pare na próxima área segura e peça o guia novamente.','หยุดในพื้นที่ปลอดภัยถัดไป แล้วขอคำแนะนำอีกครั้ง'),
('Reach this safe area immediately, then request the guide again.',
 'すぐにこの安全地帯へ移動し、ガイドをもう一度呼び出してください。','Rejoignez immédiatement cette zone sûre, puis redemandez le guide.','Raggiungi subito questa zona sicura, poi richiedi di nuovo la guida.','Erreiche diesen sicheren Bereich sofort und rufe dann die Anleitung erneut auf.','Llega a esta zona segura de inmediato y vuelve a solicitar la guía.','즉시 이 안전 지대로 이동한 뒤 가이드를 다시 요청하세요.','立即前往這個安全區域，然後再次要求指引。','立即前往这个安全区域，然后再次请求指引。','Немедленно доберитесь до этого безопасного места и снова запросите подсказку.','Chegue a esta área segura imediatamente e peça o guia novamente.','ไปยังพื้นที่ปลอดภัยนี้ทันที แล้วขอคำแนะนำอีกครั้ง'),
('Walk through the exit.',
 '出口を通ってください。','Franchissez la sortie.','Attraversa l’uscita.','Gehe durch den Ausgang.','Cruza la salida.','출구를 지나가세요.','走過出口。','走过出口。','Пройдите через выход.','Passe pela saída.','เดินผ่านทางออก'),
('Cross the falling-ceiling room from one safe area to the next.',
 '安全地帯から次の安全地帯へ移動して、天井が落ちる部屋を渡ってください。','Traversez la salle au plafond mobile d’une zone sûre à l’autre.','Attraversa la sala dal soffitto che cade passando da una zona sicura alla successiva.','Durchquere den Raum mit der fallenden Decke von einem sicheren Bereich zum nächsten.','Cruza la sala del techo que cae de una zona segura a la siguiente.','안전 지대에서 다음 안전 지대로 이동하며 내려오는 천장 방을 건너세요.','從一個安全區域移動到下一個，穿過天花板會落下的房間。','从一个安全区域移动到下一个，穿过天花板会落下的房间。','Пересеките комнату с опускающимся потолком, переходя между безопасными местами.','Atravesse a sala do teto que cai passando de uma área segura à seguinte.','ข้ามห้องเพดานถล่ม โดยเคลื่อนจากพื้นที่ปลอดภัยหนึ่งไปยังพื้นที่ปลอดภัยถัดไป')]

def main():
    path=ROOT/'data/common/mod_text.json';data=json.loads(path.read_text(encoding='utf-8'))
    assert all(len(row)==len(LANGS) for row in ROWS)
    for row in ROWS:
        key=row[0];expected=dict(zip(LANGS,row))
        assert all(text and text.count('{0}')==key.count('{0}') for text in expected.values())
        if '--check' in sys.argv:assert data.get(key)==expected,key
        else:data[key]=expected
    if '--check'not in sys.argv:path.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Roof guide:',len(ROWS),'keys in12 languages; placeholders preserved.')
if __name__=='__main__':main()
