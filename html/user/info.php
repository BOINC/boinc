<?php
require_once('../inc/util.inc');
require_once('../inc/translation.inc');

page_head(tr(RULES_TITLE));
echo "

<h3>".tr(RULES_ONLY_AUTH)."</h3>
    <p>".tr(RULES_ONLY_AUTH_TEXT)."

<h3>".tr(RULES_COMPUTER_USE)."</h3>
    <p>".tr(RULES_COMPUTER_USE_TEXT_A)."
    <p>".tr(RULES_COMPUTER_USE_TEXT_B)."

<h3>".tr(RULES_PRIVACY)."</h3>
    <p>".tr(RULES_PRIVACY_TEXT_A)."
    <p>".tr(RULES_PRIVACY_TEXT_B)."
    <p>".tr(RULES_PRIVACY_TEXT_C)."

<h3>".tr(RULES_SAFE)."</h3>
    <p>".tr(RULES_SAFE_TEXT_A)."
    <p>"; printf(tr(RULES_SAFE_TEXT_B), "<a href=\"download_network.php\">".tr(RULES_UTILITY_PROGRAM)."</a>"); echo "
    <p>"; printf(tr(RULES_SAFE_TEXT_C), COPYRIGHT_HOLDER); echo "

<h3>".tr(RULES_LIABILITY)."</h3>
    <p>"; printf(tr(RULES_LIABILITY_TEXT), COPYRIGHT_HOLDER); echo "

<h3>".tr(RULES_OTHER)."</h3>
    <p>".tr(RULES_OTHER_TEXT_A)."
    <p>".tr(RULES_OTHER_TEXT_B);
page_tail();
?>
