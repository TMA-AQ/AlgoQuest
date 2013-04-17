-- MSALGOQUEST
SELECT table_1.champ_2, Count(table_1.champ_2)
FROM table_1,table_6
WHERE 
	table_1.val_1=table_6.val_1
	AND table_1.champ_2 > 'abcabcghi'
	AND table_1.champ_2 IN 
		(
			-- level 2
			SELECT table_1.champ_2
			FROM table_1,table_6
			WHERE 
				table_1.val_1=table_6.val_1
				AND table_1.champ_2 > 'a'
				AND table_1.champ_2 < 's'
				AND table_1.champ_2 IN 
					(
						-- level 3
						SELECT table_1.champ_2
						FROM table_1,table_6
						WHERE 
							table_1.val_1=table_6.val_1
							AND table_1.champ_2 > 'd'
							AND table_1.champ_2 < 'v'
					)
		)
	AND table_1.champ_2 IN
		(
			-- level 2
			SELECT table_1.champ_2
			FROM table_1,table_6
			WHERE 
				table_1.val_1=table_6.val_1
				AND table_1.champ_2 > 'g'
				AND table_1.champ_2 < 'y'
		)
Group By table_1.champ_2
Order By table_1.champ_2
;
