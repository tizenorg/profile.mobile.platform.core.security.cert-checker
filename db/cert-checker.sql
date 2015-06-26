BEGIN EXCLUSIVE TRANSACTION;

-- Update here on every schema change! Integer value.
PRAGMA user_version = 1;

-- Table 'to_check'
CREATE TABLE IF NOT EXISTS to_check (
  check_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  app_id   TEXT NOT NULL,
  pkg_id   TEXT NOT NULL,
  uid      INTEGER NOT NULL,
  verified INTEGER NOT NULL,

  UNIQUE (app_id, pkg_id, uid) ON CONFLICT REPLACE
);

-- Table 'ocsp_urls'
CREATE TABLE IF NOT EXISTS ocsp_urls (
  issuer TEXT NOT NULL PRIMARY KEY,
  url    TEXT NOT NULL,
  date   INTEGER NOT NULL
);

-- Table 'chains_to_check'
CREATE TABLE IF NOT EXISTS chains_to_check (
  chain_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  check_id INTEGER NOT NULL,

  FOREIGN KEY (check_id) REFERENCES to_check(check_id) ON DELETE CASCADE
);

-- Table 'certs_to_check'
CREATE TABLE IF NOT EXISTS certs_to_check (
  chain_id    INTEGER NOT NULL,
  certificate TEXT NOT NULL,

  PRIMARY KEY (chain_id, certificate),
  FOREIGN KEY (chain_id) REFERENCES chains_to_check(chain_id) ON DELETE CASCADE
);

COMMIT TRANSACTION;
