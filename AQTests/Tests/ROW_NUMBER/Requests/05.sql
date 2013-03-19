-- BNP
SELECT annee,
       Row_number()
         over (
           PARTITION BY annee
           ORDER BY annee)
FROM   td_hebdo_calendrier
WHERE  annee < 2005; 