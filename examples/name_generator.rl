
alphabet := ['x','y','r','i','q','u','l','a','e','g','n', 'v', 'c', 'f'];

gen_name := function () does (
	name := '';

	while rand > 0.01 do {
		if rand > 0.5 and len name > 3 then (
			name := name[0:1] + name[3:-2]
		) else (
			name := name + global(alphabet[trunc(rand*len alphabet)])
		)
	};

	name
);

name := none;
input := 'no';
while not input = 'yes' do (
	name := gen_name();
	write ('Can you pronounce "', name, '"?\n');
	input := read split ' ' split '\n'
);

write ('My new name is "', name, '".\n')

