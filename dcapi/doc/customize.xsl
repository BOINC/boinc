<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version='1.0'>
    <xsl:import href="http://db2latex.sourceforge.net/xsl/docbook.xsl"/>

    <!-- The language defaults to French... -->
    <xsl:param name="l10n.gentext.default.language">en</xsl:param>
    <xsl:param name="latex.babel.language">english</xsl:param>
    <xsl:param name="latex.inputenc">latin1</xsl:param>

    <!-- Set the document class explicitely. -->
    <xsl:param name="latex.documentclass">book</xsl:param>
    <!-- xsl:param name="latex.documentclass.book"></xsl:param -->

    <!-- Times looks nice :-) -->
    <xsl:param name="latex.document.font">times</xsl:param>

    <!-- Turn on some extra packages -->
    <xsl:param name="latex.use.fancyvrb">1</xsl:param>
    <xsl:param name="latex.use.longtable">1</xsl:param>

    <!-- Do not use graphics for admonitions -->
    <xsl:param name="admon.graphics.path"></xsl:param>

    <!-- Put titles after the object -->
    <xsl:param name="formal.title.placement">
	figure after
	example after
	equation after
	table after
	procedure after
    </xsl:param>

</xsl:stylesheet>
