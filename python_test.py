import roulette

print(dir(roulette))

for i in range(100):
    print(f'#{i}:{roulette.random_range(0, 100)}')

randomizer = roulette.roulette([('boy', 1),('hell', 2.5),('fuckton', 2),('tick', 1), ('smelly', 1)])

randomizer.remove('tick')
randomizer.update('boy', 4.5)

counter = {}

for i in range(10000):

    value = randomizer.roll()
    # print(f'{i}#: {value}')

    if value not in counter:
        counter[value] = 1
    else:
        counter[value] += 1

print(f'randomizer has {len(randomizer)} elements')

for key,value in counter.items():
    print(f'{key}:{value}')

iter = iter(randomizer)

print(f'iter type = {type(iter)}')

for val, chance in randomizer:
    print(f'{val} has a chance of {chance}')

