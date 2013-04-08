-- MSALGOQUEST
SELECT Count (table_1.val_1)
FROM   table_1,
       table_2,
       table_3,
       table_4
WHERE  table_1.champ_1 = table_2.champ_1
       AND table_3.champ_3 = table_1.champ_3
       AND table_3.champ_2 = table_1.champ_2
       AND table_1.champ_3 = table_4.champ_3
       AND table_1.champ_2 = table_2.champ_2
       AND table_2.champ_4 = table_3.champ_4;
