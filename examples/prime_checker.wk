
is_prime := function (x) does (
	ret := true;
	for i := 2 \ i <= sqrt x and ret \ i := i+1 do (
		if x mod i = 0 then (
			ret := false
		)
	);
	ret
);

write ("Input a number: ");
number := to_num(read split ' ' split '\n');
write (number, if is_prime(number) then " is a prime.\n" else " is not a prime.\n")

