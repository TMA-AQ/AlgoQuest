-- MSALGOQUEST
-- Oracle:    2741 seconds
-- AlgoQuest: 25 seconds
-- Expected Result : 142
SELECT 
	Avg (table_1.val_1)
FROM
	table_1,
	table_2,
	table_3,
	table_4
WHERE
		table_1.champ_5 = table_2.champ_5
	AND table_3.champ_2 = table_1.champ_2
	AND table_4.champ_3 = table_3.champ_3
	AND table_3.champ_1 IN ( 'defpqrghi', 'defghidef', 'yzghiyz' ); 
