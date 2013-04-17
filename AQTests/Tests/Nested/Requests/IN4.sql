-- MSALGOQUEST
SELECT table_1.champ_2
FROM table_1, table_6
WHERE  
	table_1.val_1 = table_6.val_1
	AND table_1.champ_2 < 'abcabcghi'
	AND table_1.champ_2 IN 
		(
			-- level 2
			SELECT table_1.champ_2
			FROM table_1, table_6
			WHERE  
				table_1.val_1 = table_6.val_1
				AND table_1.champ_2 < 'a'
		)
Group By table_1.champ_2
Order By table_1.champ_2
; 