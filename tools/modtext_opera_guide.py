"""Original compact Opera guidance, authored in all twelve mod languages."""
import json
import sys
from pathlib import Path
P=Path(__file__).resolve().parents[1]/'data/common/mod_text.json'
LOCALES=('en','ja','fr','it','de','es','ko','zht','zhc','ru','pt','th')
ROWS={
'Opera lyric {0} of 3: choose option {1} of 2.':(
 'Opera lyric {0} of 3: choose option {1} of 2.','歌詞{0}/3：2つの選択肢の{1}番目を選んでください。','Paroles {0} sur 3 : choisissez l’option {1} sur 2.','Testo {0} di 3: scegli l’opzione {1} di 2.','Operntext {0} von 3: Wähle Option {1} von 2.','Letra {0} de 3: elige la opción {1} de 2.','오페라 가사 {0}/3: 두 선택지 중 {1}번을 고르세요.','歌詞第{0}段，共3段：選擇2個選項中的第{1}個。','歌词第{0}段，共3段：选择2个选项中的第{1}个。','Текст оперы {0} из 3: выберите вариант {1} из 2.','Letra {0} de 3: escolha a opção {1} de 2.','เนื้อร้องโอเปร่า {0} จาก 3: เลือกตัวเลือก {1} จาก 2'),
'Wait for the next opera cue.':(
 'Wait for the next opera cue.','次のオペラの合図を待ってください。','Attendez le prochain signal de l’opéra.','Attendi il prossimo segnale dell’opera.','Warte auf das nächste Opernsignal.','Espera la siguiente señal de la ópera.','다음 오페라 신호를 기다리세요.','等待下一個歌劇提示。','等待下一个歌剧提示。','Дождитесь следующего сигнала оперы.','Aguarde o próximo sinal da ópera.','รอสัญญาณโอเปร่าครั้งถัดไป'),
}
def main():
 original=P.read_text(encoding='utf-8');catalog=json.loads(original)
 additions={k:dict(zip(LOCALES,v)) for k,v in ROWS.items() if k not in catalog}
 for k,v in ROWS.items():assert len(v)==12 and v[0]==k and all(v)
 if '--check' in sys.argv:assert not additions,'Missing Opera guide translations'
 if additions:
  insertion=json.dumps(additions,ensure_ascii=False,indent=2)[2:-2]
  P.write_text(original[:original.rfind('}')].rstrip()+',\n'+insertion+'\n}\n',encoding='utf-8')
  catalog.update(additions)
 for k in ROWS:assert set(catalog[k])==set(LOCALES) and all(catalog[k].values())
 print(f'Opera: {len(ROWS)} instructions in twelve languages; {len(additions)} added')
if __name__=='__main__':main()
