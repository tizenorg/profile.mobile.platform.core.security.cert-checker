BEGIN EXCLUSIVE TRANSACTION;

-- Update here on every schema change! Integer value.
PRAGMA user_version = 1;

-- Table 'to_check'
CREATE TABLE IF NOT EXISTS to_check (
  check_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  app_id   TEXT NOT NULL,
  pkg_id   TEXT NOT NULL,
  uid      INTEGER NOT NULL,
  verified INTEGER NOT NULL
);

-- Table 'ocsp_urls'
CREATE TABLE IF NOT EXISTS ocsp_urls (
  url_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  issuer TEXT NOT NULL,
  url    TEXT NOT NULL,
  date   INTEGER NOT NULL
);

-- Table 'certs_to_check'
CREATE TABLE IF NOT EXISTS certs_to_check (
  cert_id     INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  check_id    INTEGER NOT NULL,
  certificate TEXT NOT NULL,

  FOREIGN KEY (check_id) REFERENCES to_check(check_id)
);

COMMIT TRANSACTION;
