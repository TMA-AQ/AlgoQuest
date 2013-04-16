-- MSALGOQUEST
SELECT table_1.champ_2, count(table_1.champ_2)
FROM table_1, table_6
WHERE 
	table_1.val_1=table_6.val_1
	AND table_1.champ_2 > 'abcabcghi'
	AND table_1.champ_2 IN 
		(
			SELECT table_1.champ_2
			FROM table_1, table_6
			WHERE 
				table_1.val_1=table_6.val_1
				AND table_1.champ_2 > 'abcabcabc'
				AND table_1.champ_2 < 'abcabcpqr'
		)
group by table_1.champ_2
;