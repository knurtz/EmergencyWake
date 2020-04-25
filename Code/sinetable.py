from math import sin, pi, pow

values = 480
minimum = 10000
maximum = int(pow(2,16)) - 10000

def sinetable(_v, _min, _max):
	_middle = (_min + _max) / 2
	_range = (_max - _min) / 2
	_foo = ""
	print("Middle: " + str(_middle) + ", Range: " + str(_range))
	for i in range(1, _v + 1):
		_foo = _foo + 2 * (str(int(_range * sin(2*pi*i/_v))) + ", ")
		if i % 10 == 0:
			print(_foo)
			_foo = ""
	print(_foo)

print("Sinetable for " + str(values) + " values from " + str(minimum) + " to " + str(maximum) + ":")
sinetable(values, minimum, maximum)

