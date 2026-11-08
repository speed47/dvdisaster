/** GLib string models used by the non-constant-format query. */
import semmle.code.cpp.models.interfaces.DataFlow
import semmle.code.cpp.models.interfaces.Taint
import semmle.code.cpp.models.interfaces.SideEffect

/** Duplicating a string preserves its contents and does not modify its input. */
class GlibStrdup extends DataFlowFunction, SideEffectFunction {
  GlibStrdup() { this.hasGlobalName("g_strdup") }

  override predicate hasDataFlow(FunctionInput input, FunctionOutput output) {
    input.isParameterDeref(0) and output.isReturnValueDeref()
  }

  override predicate hasOnlySpecificReadSideEffects() { any() }
  override predicate hasOnlySpecificWriteSideEffects() { any() }

  override predicate hasSpecificReadSideEffect(ParameterIndex i, boolean buffer) {
    i = 0 and buffer = true
  }
}

/** A bounded copy propagates taint from the source into the destination. */
class GlibStrlcpy extends TaintFunction, SideEffectFunction {
  GlibStrlcpy() { this.hasGlobalName("g_strlcpy") }

  override predicate hasTaintFlow(FunctionInput input, FunctionOutput output) {
    input.isParameterDeref(1) and output.isParameterDeref(0)
  }

  override predicate hasOnlySpecificReadSideEffects() { any() }
  override predicate hasOnlySpecificWriteSideEffects() { any() }

  override predicate hasSpecificReadSideEffect(ParameterIndex i, boolean buffer) {
    i = 1 and buffer = true
  }

  override predicate hasSpecificWriteSideEffect(ParameterIndex i, boolean buffer, boolean mustWrite) {
    i = 0 and buffer = true and mustWrite = false
  }
}

/**
 * Formatting reads the format and arguments and returns an allocated string.
 * Variadic arguments may still be written through %n; the format is read-only.
 */
class GlibStrdupPrintf extends TaintFunction, SideEffectFunction {
  GlibStrdupPrintf() { this.hasGlobalName("g_strdup_printf") }

  override predicate hasTaintFlow(FunctionInput input, FunctionOutput output) {
    exists(ParameterIndex i |
      i >= 0 and (input.isParameter(i) or input.isParameterDeref(i)) and
      output.isReturnValueDeref()
    )
  }

  override predicate hasOnlySpecificReadSideEffects() { any() }
  override predicate hasOnlySpecificWriteSideEffects() { any() }

  override predicate hasSpecificReadSideEffect(ParameterIndex i, boolean buffer) {
    i >= 0 and buffer = true
  }

  override predicate hasSpecificWriteSideEffect(ParameterIndex i, boolean buffer, boolean mustWrite) {
    i >= 1 and buffer = true and mustWrite = false
  }
}
