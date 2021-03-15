import roulette

for i in range(100):
    print(f'#{i}:{roulette.random_range(0, 100)}')

randomizer = roulette.roulette

print(type(randomizer))