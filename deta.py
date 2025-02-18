import re
indirectory = "./tools/in"
outdirectory = "./tools/out"

print("テストケース数 : ")

m_list = []
init_money_list = []

station_pos_list = []

n = int(input())
for i in range(n):
	now = f'{i:04}'
	
	with open(indirectory+"/"+now+".txt") as f:
		numbers = list(map(int, f.read().split()))
		m_list.append(numbers[1])
		init_money_list.append(numbers[2])

	with open(outdirectory+"/"+now+".txt") as f:
		text = f.read()

		# 正規表現で数字（整数）を抽出
		numbers = list(map(int, re.findall(r'-?\d+', text)))
		station_pos_list.append(numbers[0])



for i in m_list:
	print(i)

print()
for i in init_money_list:
	print(i)
print()
for i in station_pos_list:
	print(i)
print()
input()
