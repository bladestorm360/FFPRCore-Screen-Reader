"""Maintain exact reviewed ladder labels, not generic world/story overrides."""
from pathlib import Path
import hashlib,json

root=Path(__file__).resolve().parents[1];dump=root.parent/'tools/mapdump'
cases=[('Map_20301_1','地上全体へ',334,'sc_ship_0007','e42dd28d9522a308cbf13a0543ba726664964c151881d6d8fb626b382bf38e09'),
       ('Map_20351_1','浮遊大陸へ',331,'sc_ship_0004','92610a7963176c7fe18c2940aec432ee3f4171b3dd265a68e91ae5f04b71de89')]
objects=[json.loads(line)for line in (dump/'ff3_objects.jsonl').read_text().splitlines()]
scripts=[json.loads(line)for line in (dump/'ff3_scripts.jsonl').read_text().splitlines()]
path=root/'tools/world_event_reviews.json';reviews=json.loads(path.read_text())
for submap,raw,script_id,name,expected in cases:
    obj=next(row for row in objects if row['submap']==submap and row['id']==50 and row['group']=='entity_default')
    prop=obj['props'];assert (obj['type'],obj['name'],prop['entity_id'],prop['script_id'],prop['action_id'])==(2,raw,1,script_id,2)
    body=next(row for row in scripts if row['submap']==submap and row['name']==name)
    digest=hashlib.sha256(json.dumps(body,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()
    assert digest==expected
    assert {row['mnemonic']for row in body['mnemonics']}<={'Nop','Branch','PlaySE','FadeOut','SyncSE','ChangeMap','Exit'}
    review=dict(game=3,submap=submap,group='entity_default',id=50,entity_id=1,raw=raw,script_id=script_id,
                script_name=name,script_sha256=digest,label='Invincible ladder',
                reason='reviewed Invincible ladder: physical return transfer with sound/fade, no story dialogue')
    old=[row for row in reviews if (row['game'],row['submap'],row['group'],row['id'])==(3,submap,'entity_default',50)]
    assert not old or old==[review],'Preserve a conflicting earlier identity review'
    if not old:reviews.append(review)
path.write_text(json.dumps(reviews,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
catalog_path=root/'data/common/mod_text.json';catalog=json.loads(catalog_path.read_text())
catalog['Invincible ladder']={'en':'Invincible ladder','ja':'インビンシブルのはしご',
    'fr':"Échelle de l’Invincible",'it':"Scala dell’Invincibile",'de':'Leiter der Invincible',
    'es':'Escalera del Invencible','ko':'인빈서블 사다리','zht':'無敵號梯子','zhc':'无敌号梯子',
    'ru':'Лестница «Непобедимого»','th':'บันไดเรืออินวินซิเบิ้ล','pt':'Escada do Invencível'}
assert set(catalog['Invincible ladder'])==set(catalog['Ladder'])
catalog_path.write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print('Two exact Invincible ladder scripts reviewed; label localized in all12 languages')
