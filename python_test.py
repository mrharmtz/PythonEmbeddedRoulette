import roulette

for i in range(100):
    print(f'#{i}:{roulette.random_range(0, 100)}')

randomizer = roulette.roulette()

print(type(randomizer))

for i in range(10):
    randomizer.insert(f'{i}', i+1)

for i in range(10):
    print(f'{randomizer.roll()}')