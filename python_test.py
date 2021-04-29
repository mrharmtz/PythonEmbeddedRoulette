import roulette

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

