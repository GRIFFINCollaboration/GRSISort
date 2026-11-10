# Energy calibration

Energy calibration parameters are stored per channel in a GRSISort calibration
file. `TDetectorHit::GetEnergy()` passes the integrated charge to
`TChannel::CalibrateENG()`, which applies any configured charge correction and
then the energy calibration.

## Pre-calibration charge correction

The optional `ENGChargeCorr` field provides a charge correction containing an
offset, square-root coefficient, and gain, in that order:

```text
ENGChargeCorr: c0 cSqrt c1
```

For an input charge `q`, the corrected charge is

```text
q_corrected = c0 + cSqrt * sqrt(q) + c1 * q
```

The correction is applied after integration normalization and before selecting
an energy-calibration range. The existing `ENGCoeff` polynomial is then evaluated
using `q_corrected`. For example, a fifth-order calibration contains six
coefficients:

```text
ENGCoeff: a0 a1 a2 a3 a4 a5
```

```text
energy = a0 + a1 * q_corrected + a2 * q_corrected^2
       + a3 * q_corrected^3 + a4 * q_corrected^4 + a5 * q_corrected^5
```

`ENGChargeCorr` must contain exactly three coefficients. A malformed entry is
reported and ignored. The square-root term requires a non-negative normalized
charge; a negative charge produces a non-finite corrected energy.

### Compatibility

`ENGChargeCorr` is optional. When it is absent, energy calibration follows the
existing code path. The meaning and calculation of the existing `ENGDrift` and
`ENGCoeff` fields have not changed.

If both `ENGChargeCorr` and `ENGDrift` are present for a channel,
`ENGChargeCorr` takes precedence and `ENGDrift` is not applied. New calibration
files should use only one of these fields per channel.

Programs must be rebuilt against a version of GRSISort that recognizes
`ENGChargeCorr`. Older binaries skip unknown calibration fields and therefore
cannot apply this correction, although they continue to process older
calibration files normally.

## Run-specific calibrations

For run-by-run gain-drift correction, write the appropriate `ENGChargeCorr`
coefficients into each run's calibration file. The existing `WriteCalToRoot`
utility can embed the calibration without source changes:

```sh
WriteCalToRoot update run12345.cal analysis12345.root
```

Use a separate invocation for each run because one `WriteCalToRoot` invocation
applies the same calibration file to every ROOT file supplied to it. Use
`update` for a correction file that augments an existing embedded calibration.
Use `replace` only when the supplied file contains the complete calibration.

`WriteCalToRoot` must be rebuilt with the updated GRSISort libraries so that the
new field is parsed and retained in the embedded calibration.
