#pragma once
namespace ffpr::battle::scan_feedback {
// Called only after the native message is known nonempty. Outside a verified
// FFIII Scan producer, preserve the existing message path unchanged.
bool accept_message();
}
