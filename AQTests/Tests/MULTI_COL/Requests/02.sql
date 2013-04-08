-- MSALGOQUEST
SELECT 
	table_1.champ_1, 
	table_1.champ_2, 
	table_1.champ_3, 
	table_6.champ_1, 
	table_6.champ_2, 
	table_7.champ_2, 
	table_7.champ_3
FROM 
	table_1, 
	table_6, 
	table_7
WHERE 
	table_1.champ_3 = table_7.champ_2 AND
	table_1.champ_1 = table_6.champ_1 AND
	table_1.champ_2 = table_7.champ_3 AND
	table_1.champ_2 = table_6.champ_2
;