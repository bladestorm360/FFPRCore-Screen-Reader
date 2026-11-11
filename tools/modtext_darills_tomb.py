"""Three original concise tomb instructions in all twelve supported languages."""
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[1]
ROWS=[
dict(en='Raise the water, then request guidance again.',ja='水位を上げてから、もう一度ガイドを呼び出してください。',fr='Faites monter l’eau, puis redemandez le guidage.',it='Alza il livello dell’acqua, poi richiedi la guida.',de='Hebe den Wasserstand und fordere erneut Führung an.',es='Sube el nivel del agua y vuelve a pedir indicaciones.',ko='수위를 올린 후 안내를 다시 요청하세요.',zht='升高水位，然後再次請求引導。',zhc='升高水位，然后再次请求引导。',ru='Поднимите уровень воды и снова запросите подсказку.',pt='Eleve o nível da água e peça orientação novamente.',th='เพิ่มระดับน้ำแล้วขอคำแนะนำอีกครั้ง'),
dict(en='Ride the turtle, then request guidance again.',ja='カメに乗って移動してから、もう一度ガイドを呼び出してください。',fr='Traversez sur la tortue, puis redemandez le guidage.',it='Attraversa sulla tartaruga, poi richiedi la guida.',de='Fahre mit der Schildkröte und fordere erneut Führung an.',es='Cruza sobre la tortuga y vuelve a pedir indicaciones.',ko='거북이를 타고 이동한 후 안내를 다시 요청하세요.',zht='乘烏龜渡過，然後再次請求引導。',zhc='乘乌龟渡过，然后再次请求引导。',ru='Переправьтесь на черепахе и снова запросите подсказку.',pt='Atravesse na tartaruga e peça orientação novamente.',th='ขี่เต่าข้ามไปแล้วขอคำแนะนำอีกครั้ง'),
dict(en='Examine the gravestone. Choose Yes, then options 4, 1, 2, 1.',ja='墓石を調べて「はい」を選び、続く選択肢を4番、1番、2番、1番の順に選んでください。',fr='Examinez la pierre tombale. Choisissez Oui, puis les options 4, 1, 2, 1.',it='Esamina la lapide. Scegli Sì, poi le opzioni 4, 1, 2, 1.',de='Untersuche den Grabstein. Wähle Ja, dann die Optionen 4, 1, 2, 1.',es='Examina la lápida. Elige Sí y luego las opciones 4, 1, 2, 1.',ko='묘비를 조사하세요. 예를 선택한 다음 4, 1, 2, 1번 항목을 차례로 선택하세요.',zht='調查墓碑。選擇「是」，然後依序選第4、1、2、1項。',zhc='调查墓碑。选择“是”，然后依次选第4、1、2、1项。',ru='Осмотрите надгробие. Выберите «Да», затем варианты 4, 1, 2, 1.',pt='Examine a lápide. Escolha Sim e depois as opções 4, 1, 2, 1.',th='สำรวจป้ายหลุมศพ เลือกใช่ แล้วเลือกตัวเลือก 4, 1, 2, 1 ตามลำดับ')]
if __name__=='__main__':
    p=ROOT/'data/common/mod_text.json';d=json.loads(p.read_text(encoding='utf-8'))
    for row in ROWS:
        assert len(row)==12
        if'--check'in sys.argv:assert d.get(row['en'])==row
        else:assert row['en']not in d or d[row['en']]==row;d[row['en']]=row
    if'--check'not in sys.argv:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Darill: three instructions/all12 languages; native menu positions do not depend on translated inscription text.')
