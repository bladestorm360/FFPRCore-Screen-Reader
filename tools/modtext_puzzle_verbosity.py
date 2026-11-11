"""Original short guidance notices and verbosity UI, all12 supported languages."""
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
ROWS={
 'Puzzle guide verbosity':dict(en='Puzzle guide verbosity',ja='パズルガイドの詳しさ',fr='Détail du guide des énigmes',it='Dettaglio della guida agli enigmi',de='Ausführlichkeit der Rätselhilfe',es='Detalle de la guía de puzles',ko='퍼즐 안내 상세도',zht='謎題引導詳細程度',zhc='谜题引导详细程度',ru='Подробность подсказок к загадкам',pt='Detalhamento do guia de enigmas',th='ระดับรายละเอียดคำแนะนำปริศนา'),
 'Minimal':dict(en='Minimal',ja='最小限',fr='Minimal',it='Minimo',de='Minimal',es='Mínimo',ko='최소',zht='精簡',zhc='精简',ru='Кратко',pt='Mínimo',th='น้อยที่สุด'),
 'Puzzle guide verbosity help':dict(en='Minimal gives current directions and puzzle instructions. Verbose also announces guide activation and step numbers.',ja='最小限では現在の道順とパズルの操作だけを読み上げます。詳細ではガイドの開始と手順番号も読み上げます。',fr='Le mode minimal lit le trajet et les instructions actuelles. Le mode détaillé annonce aussi le démarrage du guide et les numéros des étapes.',it='Minimo legge il percorso e le istruzioni attuali. Dettagliato annuncia anche l’avvio della guida e i numeri dei passaggi.',de='Minimal liest die aktuelle Route und Rätselanweisung. Ausführlich nennt auch den Start der Hilfe und die Schrittnummern.',es='Mínimo lee la ruta y las instrucciones actuales. Detallado también anuncia el inicio de la guía y los números de los pasos.',ko='최소 모드는 현재 경로와 퍼즐 조작만 읽습니다. 상세 모드는 안내 시작과 단계 번호도 읽습니다.',zht='精簡模式只朗讀目前的路線與謎題操作。詳細模式也會朗讀引導啟動與步驟編號。',zhc='精简模式只朗读当前路线与谜题操作。详细模式还会朗读引导启动与步骤编号。',ru='Краткий режим читает текущий маршрут и действия. Подробный также объявляет запуск подсказок и номера шагов.',pt='Mínimo lê a rota e as instruções atuais. Detalhado também anuncia a ativação do guia e os números das etapas.',th='โหมดน้อยที่สุดอ่านเส้นทางและวิธีแก้ปริศนาปัจจุบัน โหมดละเอียดแจ้งการเริ่มคำแนะนำและหมายเลขขั้นตอนด้วย'),
 'Trap required':dict(en='Trap required',ja='罠通過が必要',fr='Piège requis',it='Trappola necessaria',de='Falle erforderlich',es='Trampa necesaria',ko='함정 통과 필요',zht='必須觸發陷阱',zhc='必须触发陷阱',ru='Нужна ловушка',pt='Armadilha obrigatória',th='ต้องผ่านกับดัก'),
 'Event required':dict(en='Event required',ja='イベント通過が必要',fr='Événement requis',it='Evento necessario',de='Ereignis erforderlich',es='Evento necesario',ko='이벤트 통과 필요',zht='必須觸發事件',zhc='必须触发事件',ru='Нужно событие',pt='Evento obrigatório',th='ต้องผ่านเหตุการณ์')}
if __name__=='__main__':
 p=ROOT/'data/common/mod_text.json';d=json.loads(p.read_text(encoding='utf-8'));languages=set(d['Exit to {0}']);assert len(languages)==12
 for key,row in ROWS.items():
  assert set(row)==languages and all(row.values())
  if'--check'in sys.argv:assert d.get(key)==row,key
  else:assert key not in d or d[key]==row,key;d[key]=row
 if'--check'not in sys.argv:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Puzzle guide verbosity/short route notices:five keys/all12 languages.')
