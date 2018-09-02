
fib := function (n) does (
	if (n <= 1) then
		1
	else
		global (
			fib(n-1) + fib(n-2)
		)
);

write ([{for i := 0 \ i <= 20 \ i := i+1 do fib(i)}], '\n')

