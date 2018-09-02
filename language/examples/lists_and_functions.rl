
avg := function (list) does (
	count := 0;
	sum := 0;
	while (count < len list) do (
		sum := sum + list[count];
		count := count+1
	);
	sum / len list
);

list := [1.12e+12, 2, 3, 4, 5, 6, 7, 8, 9, 0.1e-42];

write("Average of ", list, " is ", avg(list), '\n');
write("Average of ", list[1:-2], " is ", avg(list[1:-2]), '\n');
write("List contains 2 at position ", list find 2, ".\n")

