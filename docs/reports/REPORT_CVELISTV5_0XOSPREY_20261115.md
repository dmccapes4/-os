# Report — cvelistV5 and cve.0xosprey.com

**Filed:** 2026-11-15 · **Shared by:** Will Duncan (Santa Barbara City College friend of the operator; cyber security, penetration tester)
**Scope:** the local checkout at `~/dev/cvelistV5`, its upstream (CVEProject), and the live site https://cve.0xosprey.com/ (fetched and read 2026-11-15). Facts labelled measured where read directly this session.

## 1. What the repo is (measured)

`cvelistV5` is the **official CVE List** (CVEProject) — the catalogue of all CVE Records identified by or reported to the CVE Program. Local state this session:

- **7.6 GB, 425,880 JSON record files**, organised `cves/<year>/<bucket>/CVE-YYYY-NNNN.json`, updated by the official CVE Services API roughly every 7 minutes; the clone's last commits are single-CVE arrivals (`CVE-2026-129543`, …) — the feed never sleeps.
- Records are **CVE Record Format 5.2.0** (as of 2025-10-29): CNA container, CVE Program Container, optional ADP containers (CISA-ADP adds SSVC categorisation, KEV known-exploited status, and "vulnrichment" — CVSS/CWE/CPE filled in when the CNA didn't). Format 5.2 adds Package URL (PURL) identifiers.
- Distribution is git or release zips: a daily midnight **baseline** plus **hourly delta** zips. The README warns the zips "are quite large." That is an understatement with 426k records.
- **No pull requests.** It is a publication ledger, not a collaboration repo.

## 2. What the site is (measured)

`cve.0xosprey.com` is a one-page **CVE Tracker** built over this repo: dark monospace page, uPlot charts, resolution/range controls (30D–All), Published-vs-Reserved date modes, log scale, CSV/PNG export, and a live `data.json` feed (fetched: generated 2026-11-15T07:00Z, source declared as "CVEProject/cvelistV5 (PUBLISHED records, by datePublished day)", **total 406,596 published records** charted back to 1999-09). Footer links the source repo, a guestbook, and sibling projects (`mac.0xosprey.com`, 0xosprey.com home).

The name, in the operator's parsing (his, recorded): **0x** (hex prefix — the marker that tells the machine what kind of thing follows) + **OS** (operating system) + **prey** — and the whole word is **osprey**, the hawk. "The operating system preys like a hawk"; the directory `cve.` is the forward slash the attack travels on. As a pentester's site name it is honest advertising: this is a hawk-watching instrument over the vulnerability feed.

## 3. Ramifications (why a pen tester shares this)

1. **The flood is the finding.** 406,596 published CVEs and daily deltas that arrive one commit at a time mean defence is now a *stream-processing* problem. Nobody reads this list; machines triage it. The practical ramification for both sides of Will's work: offence gets a continuously refreshed target catalogue; defence gets the same catalogue as a patch queue it can never finish. Whoever's tooling reads the stream better, wins the week.
2. **Enrichment is where the value moved.** Raw CNA records are uneven — missing CVSS, missing CWE, dates historically wrong (the repo itself corrected ~27,000 records' dates in 2024, and flags a further ~515-record date discrepancy). CISA-ADP's KEV/SSVC/vulnrichment is the layer that converts the flood into priorities: *known-exploited* is the field that matters; the other 400k rows are weather.
3. **Trust is load-bearing and visibly strained.** The 2026 date-normalisation (README header: a two-week process rewriting record dates, deltaLog retention cut from 30 to 15 days to contain file size) shows even the catalogue's own metadata needs periodic repair — and every downstream metric built on those dates inherits the repair. A pentester citing a CVE's age is citing a number the programme itself has had to correct in bulk.
4. **The reserved/published gap is a shadow inventory.** The tracker's Published-vs-Reserved toggle exists because reserved-but-unpublished IDs are vulnerabilities the world has numbered but not yet described. That gap is, depending on your chair, a disclosure pipeline or a stockpile map.
5. **Machine-readability cuts both ways.** The same 7-minute delta that patches a fleet can be diffed by an attacker to learn what was just found. Publication is simultaneous for everyone; there is no defender's head start in this repo.

## 4. Assessment

As infrastructure: exemplary boring. One canonical JSON format, versioned, git-diffable, append-mostly, with its corrections announced in the README header where everyone trips over them. As a *situation*: it is the measured size of the modern attack surface, updating faster than any human process attached to it — which is precisely why a two-chart website by a working pentester is a reasonable response to it. The tracker doesn't pretend to solve the flood. It makes the flood's *rate* visible, which is the number that actually governs everyone downstream.

— HALMANSEIR, 2026-11-15
