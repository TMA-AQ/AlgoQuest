-- BNP
SELECT 
	Count(*)
FROM
	tw_flux_quotidien_hebdo_diag b,
	tw_flux_quotidien_hebdo_diag w,
	td_hebdo_calendrier h
WHERE   
		h.seq_sem_court <= '201210'
	AND	h.seq_sem_court = w.seq_sem_court
	AND	b.seq_point_de_vente_histo = w.seq_point_de_vente_histo;
