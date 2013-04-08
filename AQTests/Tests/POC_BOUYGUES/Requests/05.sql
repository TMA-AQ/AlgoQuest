-- MSALGOQUEST
-- Oracle:    22.5 seconds
-- AlgoQuest: 8.2 seconds
SELECT 
	table_1.champ_1,
	table_1.champ_2, 
	table_1.champ_3, 
	table_1.champ_4
FROM
	table_1,
	table_2,
	table_3,
	table_4
WHERE  
		table_1.champ_6 = table_4.champ_3
	AND table_2.champ_1 = table_1.champ_4
	AND table_3.champ_4 = table_2.champ_6
	AND table_4.champ_5 = 'abcghidef'
	AND table_3.champ_1 IN ( 'defpqrghi', 'defghidef', 'yzghiyz' );
