"""Original Palace guide instructions, all twelve supported languages."""
from pathlib import Path
import json,sys,re
ROOT=Path(__file__).resolve().parents[1]
TEXT={
'en':('Talk to every soldier before time runs out.','Talk to {0}. Request guidance at each choice.','Continue to {0}. Request guidance at each choice.','Choose the third option.'),
'ja':('時間切れになる前に全員の兵士に話しかけてください。','{0}に話しかけてください。選択肢ごとにガイドを呼び出してください。','{0}へ進んでください。選択肢ごとにガイドを呼び出してください。','3番目の選択肢を選んでください。'),
'fr':('Parlez à tous les soldats avant la fin du temps imparti.','Parlez à {0}. Demandez le guide à chaque choix.','Allez vers {0}. Demandez le guide à chaque choix.','Choisissez la troisième option.'),
'it':('Parla con tutti i soldati prima che scada il tempo.','Parla con {0}. Richiedi la guida a ogni scelta.','Procedi verso {0}. Richiedi la guida a ogni scelta.','Scegli la terza opzione.'),
'de':('Sprich mit allen Soldaten, bevor die Zeit abläuft.','Sprich mit {0}. Rufe bei jeder Auswahl die Hilfe auf.','Gehe weiter zu {0}. Rufe bei jeder Auswahl die Hilfe auf.','Wähle die dritte Option.'),
'es':('Habla con todos los soldados antes de que se acabe el tiempo.','Habla con {0}. Pide la guía en cada elección.','Continúa hasta {0}. Pide la guía en cada elección.','Elige la tercera opción.'),
'ko':('시간이 끝나기 전에 모든 병사와 대화하세요.','{0}에게 말을 거세요. 선택지가 나올 때마다 안내를 요청하세요.','{0}(으)로 이동하세요. 선택지가 나올 때마다 안내를 요청하세요.','세 번째 선택지를 고르세요.'),
'zht':('時間結束前與每位士兵交談。','與{0}交談。每次出現選項時請求引導。','前往{0}。每次出現選項時請求引導。','選擇第三個選項。'),
'zhc':('时间结束前与每位士兵交谈。','与{0}交谈。每次出现选项时请求引导。','前往{0}。每次出现选项时请求引导。','选择第三个选项。'),
'ru':('Поговорите со всеми солдатами до истечения времени.','Поговорите с {0}. Запрашивайте подсказку при каждом выборе.','Идите к {0}. Запрашивайте подсказку при каждом выборе.','Выберите третий вариант.'),
'pt':('Fale com todos os soldados antes que o tempo acabe.','Fale com {0}. Peça orientação em cada escolha.','Siga até {0}. Peça orientação em cada escolha.','Escolha a terceira opção.'),
'th':('คุยกับทหารทุกคนก่อนหมดเวลา','คุยกับ{0} ขอคำแนะนำทุกครั้งที่มีตัวเลือก','ไปยัง{0} ขอคำแนะนำทุกครั้งที่มีตัวเลือก','เลือกตัวเลือกที่สาม'),
}
def main():
 p=ROOT/'data/ff6/mod_text.json';d=json.loads(p.read_text(encoding='utf-8-sig'))
 assert set(TEXT)==set(json.loads((ROOT/'data/common/mod_text.json').read_text(encoding='utf-8'))['Exit to {0}'])
 for i,key in enumerate(TEXT['en']):
  row={lang:values[i]for lang,values in TEXT.items()}
  assert all(set(re.findall(r'\{\d+\}',v))==set(re.findall(r'\{\d+\}',key))for v in row.values())
  if '--check'in sys.argv:assert d.get(key)==row,key
  else:
   assert key not in d or d[key]==row,key
   d[key]=row
 if '--check'not in sys.argv:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Palace:four original instruction keys/all12 languages.')
if __name__=='__main__':main()
