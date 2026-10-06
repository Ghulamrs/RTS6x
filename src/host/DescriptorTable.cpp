// Spec: Microsoft C runtime, _setmode - the standard descriptors 0, 1 and 2 start open, in text mode.
#include "DescriptorModes.h"

namespace rts6x {

int DescriptorModes::modes_[DescriptorModes::Count] = { Text, Text, Text };

}  // namespace rts6x
