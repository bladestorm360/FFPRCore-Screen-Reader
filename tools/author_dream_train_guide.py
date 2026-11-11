"""Original reviewed train guide data; native bodies remain in external mapdump.

Imported by the maintained FFVI author. Completion uses native story/treasure
receipts. The timed lever's second pull stops the timer; no timed run is required.
"""
TWO_PULL='Examine the lever, then examine it again as soon as control returns. The second pull stops the four-second timer.'
STOP_TIMER='Examine the lever again now to stop the timer and hold the crates.'

def author(target,guide,entrance):
    switches=[target(525,n,'Examine the switch') for n in (41,32,26,10)]
    exit_=target(525,3,'Continue through the passage.');exit_['kind']='transition'
    instructions=['Open the upper-left chest.','Open the upper-middle chest.',
                  'Close the upper-right chest.','Close the lower-left chest.',
                  'Close the lower-middle chest.','Open the lower-right chest.']
    rules=[]
    def condition(bank,index,value=0,op='eq'):return [bank,index,op,value]
    def add(conditions,next_,**extra):rules.append(dict(when=conditions,targets=[next_],**extra))
    def here(map_id):return condition('map',0,map_id)
    def flag(index,value=0):return condition('scenario',index,value)
    def chest(index,value=0):return condition('treasure',index,value)
    # The native exterior flashback sets442 and retires its own event. Saved
    # states already beyond it continue from their current physical car.
    for map_id in (522,523):
        add([here(map_id),flag(442)],target(522,1,'Continue through the passage.'))
    for value,text in ((0,TWO_PULL),(1,STOP_TIMER)):
        add([here(524),chest(200),condition('local',0,value)],target(524,26,text))
    add([here(524),chest(200),condition('local',0,2,'ge')],target(524,9,'Open the chest beside the crates.'))
    add([here(524),chest(200,1),flag(444)],target(524,7,'Open the chest at the left end of this car.'))
    add([here(524),chest(200,1),flag(444,1),flag(443)],
        target(524,107,'Use the item from the opened chest on the moving chest.'))
    add([here(524),chest(200,1),flag(443,1),chest(199)],
        target(524,8,'Open the chest that the moving chest was guarding.'))
    # Enter an actual PointIn before relying on the new car's local state.
    # Treasure receipts persist across car reloads; map-local crate/chest
    # animation state does not. A completed crate reward needs no redo.
    for map_id in (522,523,526):
        for pending in ([chest(200)],[chest(200,1),flag(443)],[chest(200,1),flag(443,1),chest(199)]):
            add([here(map_id),flag(442,1),*pending],entrance(524,1))
    for map_id in (522,523,524,526):
        ready=[here(map_id),flag(442,1),chest(200,1),flag(443,1),chest(199,1)]
        add([*ready,flag(448)],entrance(525,1))
        add([*ready,flag(448,1)],entrance(527,1))
    # The locomotive levers animate only. Native event8 is the real departure;
    # arrival528/intro449, never standing beside a control, completes this guide.
    add([here(527)],target(527,8,'Continue through the passage.'),finish_on_exit=528)
    result=guide('dream_train',list(range(522,528)),'Follow the train puzzle. Use Shift+G after each action.',rules)
    result.update(complete_scenario=449,finish_on_completion=True)
    result['_about']='Native two-pull timer stop, actual treasure receipts200/199 and story443/444, complete maze/chest sequence, real departure528/449. No reward-name disclosure or locomotive lever combination. docs/re/DREAM_TRAIN_20261031.md.'
    result['maze_targets']=switches+[exit_]
    result['pattern_targets']=[target(525,61+i,text) for i,text in enumerate(instructions)]
    return result
