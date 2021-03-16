import roulette

for i in range(100):
    print(f'#{i}:{roulette.random_range(0, 100)}')

randomizer = roulette.roulette()

for i in range(10):
    randomizer.insert(f'{i}', i+1)

randomizer.remove('5')

counter = {}

for i in range(10000):

    value = randomizer.roll()
    # print(f'{i}#: {value}')

    if value not in counter:
        counter[value] = 1
    else:
        counter[value] += 1

for key,value in counter.items():
    print(f'{key}:{value}')