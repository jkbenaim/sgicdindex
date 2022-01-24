.headers on
.mode csv
.once index.csv
select discs.disc_id, products.name,cd_pn,case_pn,date,discs.name,note,contributor,date_added from discs,products where discs.product_id=products.product_id order by products.ordinal,products.name,discs.ordinal,discs.name;
