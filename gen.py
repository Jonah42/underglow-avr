def gen(x):
	s = f"uint16_t hello[{x}] = "+"{"

	for i in range(x):
		s+=f"{i},"
	s+="};"
	print(s)
gen(240)