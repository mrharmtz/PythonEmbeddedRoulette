import roulette
import os
import psutil
process = psutil.Process(os.getpid())
print(f'pid = {os.getpid()}')
start_rec = str(process.memory_info()) 

print(dir(roulette))

to_remove = 'tick'
to_update = 'boy'
initial_list = [(to_update, 1), ('hell', 2.5)]
additional_insert = [('fuckton', 2), (to_remove, 1), ('smelly', 1)]

randomizer = roulette.roulette(initial_list)

print(f'---before additional insert of {additional_insert}---')
for val, chance in randomizer:
    print(f'{val} has a chance of {chance}')

randomizer.insert_list(additional_insert)

print(f'---after additional insert of {additional_insert} before removing {to_remove}---')
for val, chance in randomizer:
    print(f'{val} has a chance of {chance}')

# randomizer.remove(to_remove)
del randomizer[to_remove]

print(f'---after removing {to_remove}, before updating chance of {to_update}---')
for val, chance in randomizer:
    print(f'{val} has a chance of {chance}')

# randomizer.update(to_update, 4.5)
randomizer[to_update] = 4.5

print(f'---after updating chance of {to_update}---')
for val, chance in randomizer:
    print(f'{val} has a chance of {chance}')

print(f'--- the chance to run {to_update} is {randomizer[to_update]}')

counter = {}

print(f'---rolling---')
for i in range(10000):

    value = randomizer.roll()

    if value not in counter:
        counter[value] = 1
    else:
        counter[value] += 1

print(f'randomizer has {len(randomizer)} elements')

for key, value in counter.items():
    print(f'{key}:{value}')

iter = iter(randomizer)

print(f'iter type = {type(iter)}')

for val, chance in randomizer:
    print(f'{val} has a chance of {chance}')

mid_rec = str(process.memory_info())

for i in range(1000):
    temp = roulette.roulette(initial_list)
    del temp

print(f'before any allocation memory usage: {start_rec}')  # in bytes 
print(f'before allocation test memory usage: {mid_rec}')  # in bytes
print(f'after allocation test memory usage: {process.memory_info()}')  # in bytes
