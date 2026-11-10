# Structure Access native ownership audit

## Provenance and complete native inventory

Baseline: `a84e9884c70f02ccc9132462cef4fc521feecbee` on `master`.
Canonical guidance: `miyako/skills` at
`e6b492fcb81e5e1a7f7db723594b420e63bf6eb9`,
`4d-skills/skills/4dplugin/SKILL.md`, fetched as immutable API content.
Harness contract prior art: `miyako/4d-plugin-script` at
`c13f12088f8c88b2f0fa6f5e79104759707ec7e2`; results below are new,
target-specific measurements, not copied results.

The root and prebuilt-bundle manifests both declare exactly one command,
`CREATE ELEMENTS FROM XML(&T)` (one scalar text input, no array output).
`4DPlugin.cpp` dispatches selector 1 directly to
`PA_CreateElementsFromXMLDefinition`; that SDK helper forwards the borrowed
input descriptor to `EX_CREATE_ELEMENTS`. There is no local allocation,
transfer, disposal, callback, object, or collection handoff in this command.
The host test intercepts the XML selector before any structure operation.
It never parses/applies a definition or opens a database.

Both `Structure Access.vcxproj` and
`Structure Access.xcodeproj/project.pbxproj` compile the complete SDK,
`4DPlugin.cpp`, and every one of these 16 wrapper translation units
(each has a corresponding header in `4D Plugin API/Classes`):

| Wrapper | Owner / string interaction | Audit disposition |
|---|---|---|
| ARRAY_BOOLEAN | Heap vector of bits | Disable shallow owner copying |
| ARRAY_DATE | Heap vector of by-value C_DATE | Disable shallow owner copying |
| ARRAY_INTEGER | Heap vector of shorts | Disable shallow owner copying |
| ARRAY_LONGINT | Heap vector of ints | Disable shallow owner copying |
| ARRAY_REAL | Heap vector of doubles | Disable shallow owner copying |
| ARRAY_TEXT | Heap vector of independent UTF-16 strings; engine array transfers | Disable shallow owner copying; keep accepted transfer cleanup unchanged |
| ARRAY_TIME | Heap vector of ints | Disable shallow owner copying |
| C_BLOB / CBytes | Raw-owned CBytes; by-value byte vector; text conversions | Disable C_BLOB copying; CBytes remains value-copyable |
| C_DATE | Value-only date fields | Unchanged |
| C_INTEGER | Value-only short | Unchanged |
| C_LONGINT | Value-only int | Unchanged |
| C_PICTURE / CPicture | Raw-owned CPicture/CBytes; retained engine picture; command arguments | Fix path, overwritten empties, creation references, return cleanup, format units, copying |
| C_POINTER | Borrowed pointer; constructs command references | Use exact SDK reference constructor, not owning value getter |
| C_REAL | Value-only double | Unchanged |
| C_TEXT | Heap UTF-16 string; content setters, independent conversions | Existing deleted copy operations are prior art and remain unchanged |
| C_TIME | Value-only time | Unchanged |

`C_TYPES.h` supplies aliases/platform imports. There are no additional
tracked native translation units, alternate wrapper copies, object/collection
helpers, or uncompiled native sources in the baseline inventory. All wrappers
are compiled but dormant from the sole exported command. No repository
consumer copies these owners; the copy defects are latent API hazards,
not observed production call paths. Ten additional owning classes receive
the same deleted-copy policy already present in C_TEXT; no data layout or
SDK ABI changes are made. All actual consumers compile together.

SDK file identities are git blob IDs, not inferred upstream release IDs:

| Protected bundled SDK file | Baseline blob |
|---|---|
| 4DPluginAPI.c | `478081f0238f6bc5393222f574bc2656f010b67b` |
| 4DPluginAPI.h | `9054b45ffc7ff6f5b8d57529a3788a55d977b314` |
| EntryPoints.h | `730f2f77ff2713fb2fe4b7d6562baeedf5f664cb` |
| Flags.h | `10a83185333dac06a62dda61b9f9428243d1e2ee` |
| PrivateTypes.h | `77053dc0c0a3718d2d3b6f158371a6af78c6ae66` |
| PublicTypes.h | `2bb392e73e9c556c274fe0a75b3a6b83e310b77c` |
| 4DPluginAPI.def | `996ecc4ab11ad165de9192eba2e7478db141c757` |

The implementation banner says v14; the header banner says v13.
All seven files remain unchanged and the runner verifies their hashes.
Wrapper provenance is the same baseline commit and its
`git ls-tree -r a84e9884c70f02ccc9132462cef4fc521feecbee -- "4D Plugin API/Classes"`
inventory, including all 16 source/header pairs and C_TYPES.h.

## Source-derived contracts and surgical changes

Baseline SDK line numbers:

| API / lines in 4DPluginAPI.c | Ownership consequence |
|---|---|
| PA_CreateUnistring / 244; PA_DisposeUnistring / 252 | Creation owner; disposal frees allocation and resets only that descriptor |
| PA_SetUnistring / 289 | Engine-backed content replacement, not descriptor transfer |
| PA_CreateVariable / 3523 | String/picture kinds allocate empty owners; Undefined does not |
| PA_SetStringVariable / 3826; PA_SetPictureVariable / 3852 | Shallow overwrite; no disposal of old owner or independent retain |
| PA_ClearVariable / 3947 | Engine cleanup; resets SDK error, so not an error-preservation mechanism |
| PA_GetStringInArray / 4266 | Borrowed element alias; copy before mutation/resize/clear |
| PA_SetStringInArray / 4444 | Accepted assignment disposes old element then transfers allocation by shallow assignment |
| PA_DuplicatePicture / 447 | retainOnly=1 maps to engine fParam1=0: retain current picture |
| PA_GetPictureVariable / 3730 | Borrowed picture from variable; retain before clearing argument |
| PA_ExecuteCommandByID / 5257 | Successful return is owning; failed return is unspecified; handle failure leaves raw fType=0 |
| PA_ConvertCharsetToCharset / 4776 | Source/destination capacities and result count are bytes, not UTF-16 units |
| PA_SetVariableOrFieldReference / 3923 | Exact SDK scope-to-reference metadata mapping, no owning value fetch |

ARRAY_TEXT copies borrowed input contents into CUTF16String before output
mutation. On successful allocation/resize it creates a fresh owned string for
each element and transfers it without post-insertion disposal. This is correct;
no blanket disposal removal was applied. Exact setter acceptance is Unicode
array kind, non-null data handle, index 0 through logical count inclusive, and
non-null incoming descriptor. Rejection retains the caller's ownership.
Index zero is tested, not excluded.

CPicture path input now starts Undefined before adopting its created string.
Argument cleanup is the only disposer of that string. The picture output is
retained by setPicture before its argument owner is cleared. setBytes releases
its successful creation reference after setPicture retains it. Picture inputs
that are subsequently assigned start Undefined to avoid abandoned empty owners;
output-only picture arguments still start with real empty picture values.
createCopy divides returned conversion bytes by sizeof(PA_Unichar) before
constructing the bounded CUTF16String.

All five command calls use one helper that captures PA_GetLastError immediately,
before any kind inspection or cleanup. Only successful returned values are
cleared, including string/picture values unrelated to the picture argument.
Failed unspecified returns are not adopted or cleared. No worker exceptions,
new error channels, or changed command failure policy were introduced.
Borrowed picture inputs for size/thumbnail are detached before argument cleanup;
owned converted/grayscale/thumbnail outputs are detached and handed to the caller.

C_POINTER previously fetched an owning value, leaked it and interpreted its
value union as reference metadata. The variable-reference branch now invokes
the exact bundled SDK constructor. The existing field-reference branch is
unchanged. Valid SDK pointer scopes 0..3 are tested against tags, indices,
names and scope-to-kind mapping; no invented metadata contract is used.

## Isolated evidence and reproduction

Run on macOS, with the existing compiler/frameworks only:

```sh
python3 .github/tests/test_ownership.py -v
```

For comparison, export baseline native sources into a named scratch directory
with `git archive` and run the **same final harness**, changing only
`OWNERSHIP_SOURCE_ROOT` to that directory. The baseline export contains the
complete SDK, all wrapper sources/headers and actual 4DPlugin.cpp/4DPlugin.h;
it contains no database/project/private-data files. No fragments or replacement
SDK functions are used.

Final harness SHA-256 identities used for both measurements:

```text
7c4fae24c2eee36d2f17bc0ac8bfc95f7e7b63cff6771b3c79db6735ded7be6c  ownership_host.mm
608964815e2393c0319c947fb724ae7af97e2d33bfa7e3f6bcd4ba2f1b0a260a  ownership_platform.h
13c3048f051982c5bf18468d33c6c2fa939c73b7aa11f2229c6791ed260d3253  test_ownership.py
```

One executable links the complete unchanged SDK, all 16 actual wrapper sources,
and actual dispatcher. It uses ASan/UBSan, with alignment instrumentation
disabled for the SDK's legacy packed ABI structs. LeakSanitizer is disabled on
macOS; explicit allocation and reference ledgers assert final strings=0,
pictures=0, handles=0, with intermediate retain/return/parameter balances.
Objects/collections are explicitly n/a, not simulated as unowned successes.

| Final harness group | Leaf runs | Baseline passed / failed | Fixed passed / failed |
|---|---:|---:|---:|
| Actual XML dispatcher, four texts x two empty representations | 8 | 8 / 0 | 8 / 0 |
| Array transfer/replacement/shrink/clear/rejection and scalar lifetime | 10 | 10 / 0 | 10 / 0 |
| Owning copy construction / fixed noncopy traits plus value controls | 12 | 2 / 10 | 12 / 0 |
| Path/copy, ASCII/BMP/supplementary/empty x allocated/null empties | 16 | 0 / 16 | 16 / 0 |
| Picture bytes, five commands x three return kinds, ten failure paths | 26 | 0 / 26 | 26 / 0 |
| Variable / field reference metadata | 2 | 1 / 1 | 2 / 0 |
| SDK failure-predicate characterization | 1 | 1 / 0 | 1 / 0 |
| **Total (seven unittest methods)** | **75** | **22 / 53** | **75 / 0** |

Named baseline failures: path duplicate/unowned string disposal (including
null-empty path's reference imbalance); createCopy ASCII heap-buffer-overflow
in CUTF16String construction; overwritten empty string/picture owner imbalances;
setBytes retained-reference imbalance; variable-reference construction calling
the owning getter; nine owner-copy ASan heap-use-after-free reports
(ARRAY_TEXT, C_PICTURE, C_BLOB, ARRAY_BOOLEAN, ARRAY_DATE, ARRAY_INTEGER,
ARRAY_LONGINT, ARRAY_REAL, ARRAY_TIME) and one CPicture duplicate-picture
disposal ledger rejection. C_TEXT noncopyability and value-only copying pass
unchanged. Fixed traits prohibit both construction and assignment.

Fixtures check exact content, UTF-16 unit length, terminators, conversion byte
capacity and returned-byte interpretation. Array fixtures include allocator
churn, repeated replacement, element zero, rejection for kind/index/data/null
descriptor, retained rejected owner, borrowed-copy independence, shrink,
clear and Undefined destination. Scalar outputs survive input mutation and
wrapper destruction. Command results independently own string/picture outputs.
Failure tests inject argument-handle allocation failure and poisoned failed
command returns before cleanup; they preserve legacy argument/output behavior
without treating those outputs as successful operations.

All engine selectors are deny-by-default. XML, pointer metadata, handles,
picture creation and picture commands are memory-only synthetic fixtures.
NSURL/NSImage/CFURL boundaries are replaced before compilation, including
dormant methods; an `nm -u` import gate rejects their real platform symbols
and process/shell/AppleScript boundaries. No real file, schema, table, record,
query, command or database operation is performed. Foundation/CoreFoundation
string conversion uses memory-only fixtures.

## Limits, deferred contracts and automation safety

This is not a Windows native ABI build, a 4D host execution, or a reproduced
Windows 4D crash. Both platform projects' consumers are compiled as macOS
Objective-C++ actual sources; Windows-specific conversion branches are not
validated by that. The actual XML dispatcher is exercised rather than a
macOS no-op. No native project build was run.

Successful PA_GetPictureData metadata ownership is not stated by this bundled
SDK; do not infer disposal from its name. Successful metadata enumeration and
loadImageOfType are deferred pending an authoritative host contract. The shim
uses an error sentinel and proves it is not read/cleared after failure, not
that successful metadata is leak-free.

Allocation/resize failure safety is not certified. The unchanged SDK updates
array logical counts despite a failed physical handle resize; the test proves
this discrepancy, repairs only the synthetic descriptor for cleanup, and does
not send that inconsistent descriptor through ARRAY_TEXT. SDK creation failure,
converter failure, invalid pointer scope, and engine argument mutation on
failed commands have no proven preservation contract here. In particular,
the legacy setBytes failure path can still pass a failed creation result to
setPicture; only the **successful** creation reference release is fixed.
Do not treat these gaps or legacy failure outputs (empty/retained picture,
zero dimensions) as success-shaped recovery guarantees. No broad catches,
new throws or fallback API were added.

Both existing workflows and all release scripts were read before validation
or push. Build and Bump version are workflow_dispatch-only and were not
dispatched. Xcode has Headers/Resources/CopyFiles/Sources/Frameworks/Rez phases,
and project output can touch the bundled plugin; these phases were bypassed
entirely. No project, manifest, version, existing workflow, prebuilt binary,
signing, notarization, packaging, release, tag, artifact upload, dependency
installation, deployment or private-data access is part of this audit.
The new Native ownership workflow has read-only contents permission, exact
feature/master branch filters and relevant native/test path filters, runs only
this unsigned synthetic test, and publishes nothing.
