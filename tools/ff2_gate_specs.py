"""Original finite guide definitions; native bodies remain external."""
def control(map_id,entity,script,flag,text,arg='',**extra):
    return dict(map=map_id,entity=entity,script=script,flag=flag,text=text,arg=arg,**extra)

QUEEN='Speak to the queen and learn {0}. Repeat the guide after learning it.'
LEARN='Ask about {0} and learn {1}. Repeat the guide after learning it.'
RECIPES=[
 ('fynn_incantation',[43],[
  control(43,25,'sc_e_0058',52,'Defeat the guardian.',receipt_script='sc_e_0058_1'),
  control(43,23,'sc_e_0116',None,QUEEN,'modtext:FF2 keyword Mysidia',keyword=924),
  control(43,23,'sc_e_0116',None,LEARN,'modtext:FF2 keyword Mysidia',keyword=930,arg2='modtext:FF2 keyword Mask'),
  control(43,23,'sc_e_0116',None,LEARN,'modtext:FF2 keyword Mask',keyword=931,arg2='modtext:FF2 keyword Ekmet Teloess'),
  control(43,22,'sc_e_0063',56,'Ask about {0} at the secret wall.','modtext:FF2 keyword Ekmet Teloess')]),
 ('kashuan_sunfire',list(range(149,164)),[
  control(149,30,'sc_e_0030',27,'Use {0} on the locked door.','modtext:FF2 item Goddess Bell'),
  control(156,12,'sc_e_0031',28,'Speak to your ally.'),
  control(150,9,'sc_e_0032',29,'Defeat the guardian.',receipt_script='sc_e_0032_1'),
  control(150,6,'sc_e_0033',30,'Open the chest.'),
  control(149,56,'sc_e_0034',31,'Use {0} on the flame.','modtext:FF2 item Egil Torch')]),
 ('mysidia_masks',[98,207],[
  control(98,4,'sc_e_0065',58,'Use {0} on the statue.','modtext:FF2 item White Mask',allow_world_transit=True),
  control(207,28,'sc_e_0068',61,'Use {0} on the doppelganger.','modtext:FF2 item Black Mask',allow_world_transit=True)]),
 ('dreadnought_escape',list(range(164,176)),[
  control(164,42,'sc_e_0036',33,'Use {0} at the guard, or ask about {1} and defeat him.','modtext:FF2 item Pass',arg2='modtext:FF2 keyword Wild Rose'),
  control(169,28,'sc_e_0199',100,'Open the cell bars.'),
  control(169,17,'sc_e_0038',34,'Speak to the prisoners to free them.'),
  control(175,12,'sc_e_0039',35,'Use {0} at the engine, then wait for the escape to finish.','modtext:FF2 item Sunfire',receipt_map=1,receipt_script='sc_e_0039_8')]),
 ('fynn_treasure_passage',[21],[
  control(21,31,'sc_e_0128',88,'Ask about {0}.','modtext:FF2 keyword Cyclone',before_scenario=75),
  control(21,36,'sc_e_0216',114,'Examine the hidden passage.')]),
 ('semitt_snowcraft',[118],[
  control(118,14,'sc_e_0025',22,'Examine the hidden passage.')]),
 ('coliseum_escape',list(range(191,196)),[
  control(191,19,'sc_e_0054',48,'Defeat the guardian.',receipt_map=192,receipt_script='sc_e_0054_2'),
  control(192,29,'sc_e_0055',49,'Examine the cell bars and wait for your ally.'),
  control(195,8,'sc_e_0056',50,'Examine the cell bars to free the prisoner.')]),
 ('fynn_mirror',[41],[
  control(41,5,'sc_e_0080',73,'Use {0} at the mirror, then wait for the scene to finish.','modtext:FF2 item Pendant',receipt_script='sc_e_0080_2')])]

REQUIRES={'fynn_incantation':51,'fynn_treasure_passage':72,
 'semitt_snowcraft':21,'coliseum_escape':47,'fynn_mirror':72}
