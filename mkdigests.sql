CREATE TEMP TABLE myhashes(
	disc_id INT,
	hashtype TEXT,
	hash TEXT,
	filename TEXT
);

INSERT INTO myhashes(
	disc_id,
	hashtype,
	hash,
	filename
) SELECT
	disc_id,
	hashtype,
	hash,
	NULL
FROM hashes;

UPDATE myhashes SET filename=(SELECT filename FROM discs WHERE discs.disc_id=myhashes.disc_id);
UPDATE myhashes SET filename=(SELECT name || '.iso' FROM discs WHERE discs.disc_id=myhashes.disc_id) WHERE filename IS NULL;

.headers off
.mode list
SELECT 
	hashtype || ' (' || filename || ') = ' || hash
FROM myhashes ORDER BY filename,hashtype;

DROP TABLE temp.myhashes;
