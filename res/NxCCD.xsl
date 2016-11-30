<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:n3="http://www.w3.org/1999/xhtml" xmlns:n1="urn:hl7-org:v3" xmlns:n2="urn:hl7-org:v3/meta/voc" xmlns:voc="urn:hl7-org:v3/voc" xmlns:sdtc="urn:hl7-org:sdtc" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <xsl:output method="html" indent="yes" version="4.01" encoding="ISO-8859-1" doctype-public="-//W3C//DTD HTML 4.01//EN"/>

  <!--<xsl:key name="ObjectIDKey" match="//n1:id" use="n1:root"/>-->


  <!-- CDA document -->
  <xsl:param name="BackgroundColorParam"/>
  <xsl:param name="HighlightColorParam"/>

  <xsl:variable name="BackgroundColor">
    <xsl:choose>
      <xsl:when test="$BackgroundColorParam!=''">
        <xsl:value-of select="$BackgroundColorParam"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>#FFFFFF</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="HighlightColor">
    <xsl:choose>
      <xsl:when test="$HighlightColorParam!=''">
        <xsl:value-of select="$HighlightColorParam"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>#F1F5EF</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  
  <xsl:variable name="title">
    <xsl:choose>
      <xsl:when test="/n1:ClinicalDocument/n1:title and /n1:ClinicalDocument/n1:code/@displayName!=''">
        <xsl:value-of select="/n1:ClinicalDocument/n1:title"/>
        <xsl:text> - </xsl:text>
        <xsl:value-of select="/n1:ClinicalDocument/n1:code/@displayName"/>
      </xsl:when>
      <xsl:when test="/n1:ClinicalDocument/n1:title">
        <xsl:value-of select="/n1:ClinicalDocument/n1:title"/>
      </xsl:when>
      <xsl:when test="/n1:ClinicalDocument/n1:code/@displayName">
        <xsl:value-of select="/n1:ClinicalDocument/n1:code/@displayName"/>
      </xsl:when>
      <xsl:otherwise>Clinical Document</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>


  <xsl:template match="/n1:ClinicalDocument">
    <html>
      <head>
        <title xml:space="preserve">
			<xsl:value-of select="$title"/>
		</title>
        <style type="text/css">

          body {
          padding: 2px;
          margin: 0px;
          font-family: Calibri, Arial, Sans-Serif;
          background-color: <xsl:value-of select="$BackgroundColor"/>;
          }

          a.TopLink {
          float:right;
          font-size: .8em;
          }

          a, a:link, a:visited, a:active {
          color: #0066CC;
          }
          a:hover {
          color: #0077DD;
          }

          h1 {
          margin: 0px;
          }

          h2 {
          margin-bottom: 0px;
          }

          div.PatientDetail, div.PatientAssociated, div.PatientPerformer {
          margin-bottom: 10px;
          }

          h3 {
          margin: 0px;
          background-color: <xsl:value-of select="$HighlightColor"/>;
          }

          div.section {
          margin-bottom: 10px;
          }

          div.section h3 {
          margin-left: 3px;
          }

          div {
          margin-left: 5px;
          }

          div div {
          border-left: 1px solid #C0C0C0;
          }

          div.content {
          border-width: 0px;
          }

          div div:hover {
          background-color: <xsl:value-of select="$HighlightColor"/>;
          }

          table {
          width:100%;
          }

          td, th {
          vertical-align:top;
          text-align:left;
          }

          .LabelCell {
          font-weight: bold;
          white-space: nowrap;
          }

          a {
          text-decoration: none;
          }



          .section td {
          border-top: 1px dotted #C0C0C0;
          }
          .section td, .section th {
          border-right: 1px dotted #C0C0C0;
          }

        </style>

        <style type="text/css">
          #menu {
          position: absolute;
          top: 45px;
          left: 0px;
          z-index: 1;
          float: left;
          text-align: right;
          color: #000;
          list-style: none;
          line-height: 1;
          border-width: 0px;
          }
        </style>

        <xsl:comment>
          <![CDATA[[if lt IE 7]>
<style type="text/css">
#menu {
	display: none;
}
</style>
<![endif]]]>
        </xsl:comment>

        <style type="text/css">

          #menu ul {
          list-style: none;
          margin: 0;
          padding: 0;
          width: 14em;
          float: right;
          text-align: right;
          color: #000000;
          }

          #menu a, #menu h2 {
          font: bold 9pt calibri, arial, helvetica, sans-serif;
          text-align: right;
          display: block;
          border-width: 0px;
          margin: 0;
          padding: 2px 3px;
          color: #000000;
          }

          #menu h2 {
          text-align: right;
          }

          #menu a {
          text-decoration: none;
          text-align: right;
          border-width: 1px;
          border-style: solid;
          border-color: <xsl:value-of select="$BackgroundColor"/> #C0C0C0 #C0C0C0 <xsl:value-of select="$BackgroundColor"/>;
          }

          #menu a:hover {
          color: #0066CC;
          background: <xsl:value-of select="$HighlightColor"/>;
          text-align: right;
          }

          #menu li {
          position: relative;
          }

          #menu ul ul {
          position: relative;
          z-index: 500;
          text-align: left;
          color: #000000;
          background-color: <xsl:value-of select="$BackgroundColor"/>;
          float: right;
          }

          #menu ul ul ul {
          position: absolute;
          top: 0;
          left: 100%;
          text-align: right;
          float: right;
          }

          div#menu ul ul,
          div#menu ul li:hover ul ul,
          div#menu ul ul li:hover ul ul
          {display: none;}

          div#menu ul li:hover ul,
          div#menu ul ul li:hover ul,
          div#menu ul ul ul li:hover ul
          {display: block;}

        </style>

      </head>
      <body xml:space="preserve">

<!-- table of contents menu -->
<div id="menu"><center>
	<table border="0" cellspacing="0" cellpadding="0" width="100%">
		<tr>
			<td>&#160;</td>
			<td width="100%">
				<ul>
					<li><h2>TOC [<font color="#000000">+</font>]</h2>
						<ul>
							<xsl:for-each select="n1:component">
								<xsl:for-each select="n1:structuredBody">
									<xsl:for-each select="n1:component">
										<xsl:for-each select="n1:section">
										<li><a><xsl:attribute name="href">#<xsl:value-of select="n1:title"/></xsl:attribute><xsl:value-of select="n1:title"/></a>
											</li>
										</xsl:for-each>
									</xsl:for-each>
								</xsl:for-each>
							</xsl:for-each>
						</ul>
					</li>
				</ul>
			</td>
			<td>&#160;</td>
		</tr>
	</table>
</center>
</div>
  
<!-- the document begins! -->
<h1>
  <a name="top"/>
  <xsl:value-of select="$title"/>
</h1>

<!--Document Information -->
<div>
  <h2><xsl:text>Document Information</xsl:text></h2>

  <div>
    <table>
      <tr>
        <td>
          <table>
	          <xsl:for-each select="n1:author/n1:assignedAuthor">
		          <xsl:if test="n1:assignedPerson/n1:name!=''">
			          <tr>
				          <td class="LabelCell">
					          <xsl:text>Author:</xsl:text>
				          </td>
				          <td>
					          <xsl:apply-templates select="n1:assignedPerson/n1:name"/>
				          </td>
			          </tr>
		          </xsl:if>
		          <xsl:if test="n1:addr!=''">
			          <tr>
				          <td class="LabelCell">
					          <xsl:text>Address:</xsl:text>
				          </td>
				          <td>
					          <xsl:apply-templates select="n1:addr"/>
				          </td>
			          </tr>
		          </xsl:if>
		          <xsl:for-each select="n1:telecom">
			          <tr>
				          <td class="LabelCell">
					          <xsl:apply-templates select="." mode="Label"/><xsl:text>:</xsl:text>
				          </td>
				          <td>
					          <xsl:apply-templates select="."/>
				          </td>
			          </tr>
		          </xsl:for-each>
		          <xsl:if test="n1:representedOrganization/n1:name!=''">
			          <tr>
				          <td class="LabelCell">
					          <xsl:text>Organization:</xsl:text>
				          </td>
				          <td>
					          <xsl:apply-templates select="n1:representedOrganization/n1:name"/>
				          </td>
			          </tr>
		          </xsl:if>
		          <xsl:if test="n1:assignedAuthoringDevice!=''">
			          <tr>
				          <td class="LabelCell">
					          <xsl:text>System:</xsl:text>
				          </td>
				          <td>
					          <xsl:apply-templates select="n1:assignedAuthoringDevice/n1:manufacturerModelName" mode="DisplayGiven"/>
                    <xsl:apply-templates select="n1:assignedAuthoringDevice/n1:softwareName" mode="DisplayGiven"/>
				          </td>
			          </tr>
		          </xsl:if>
		          <xsl:if test="n1:id/@root!=''">
			          <tr>
				          <td class="LabelCell">
					          <xsl:text>Document ID:</xsl:text>
				          </td>
				          <td>
					          <xsl:value-of select="n1:id/@root"/>
				          </td>
			          </tr>
		          </xsl:if>
	          </xsl:for-each>
	          <xsl:if test="/n1:ClinicalDocument/n1:author/n1:time">
		          <tr>
			          <td class="LabelCell">
				          <xsl:text>Time:</xsl:text>
			          </td>
			          <td>
				          <xsl:apply-templates select="/n1:ClinicalDocument/n1:author/n1:time" mode="Date"/>
			          </td>
		          </tr>
	          </xsl:if>							
	          <xsl:if test="/n1:ClinicalDocument/n1:custodian/n1:assignedCustodian/n1:representedCustodianOrganization/n1:name!=''">
		          <tr>
			          <td class="LabelCell">
				          <xsl:text>Custodian:</xsl:text>
			          </td>
			          <td>
				          <xsl:apply-templates select="/n1:ClinicalDocument/n1:custodian/n1:assignedCustodian/n1:representedCustodianOrganization/n1:name"/>
			          </td>
		          </tr>
	          </xsl:if>		
            <tr>
		          <td class="LabelCell">
			          <xsl:text>Period:</xsl:text>
		          </td>
		          <td>
                <xsl:apply-templates select="//n1:ClinicalDocument/n1:documentationOf/n1:serviceEvent/n1:effectiveTime" mode="DateNoWeekday"/>
		          </td>
	          </tr>
          </table>
        </td>
        <td>
          <table>
	          <xsl:if test="n1:title!=''">
		          <tr>
			          <td class="LabelCell">
				          <xsl:text>Title:</xsl:text>
			          </td>
			          <td>
				          <xsl:apply-templates select="n1:title"/>
			          </td>
		          </tr>
	          </xsl:if>
	          <xsl:if test="count(n1:code)&gt;0">
		          <tr>
			          <td class="LabelCell">
				          <xsl:text>Description:</xsl:text>
			          </td>
			          <td>
				          <xsl:for-each select="n1:code">
					          <xsl:apply-templates select="." mode="CE"/><br/>
				          </xsl:for-each>
			          </td>
		          </tr>
	          </xsl:if>
	          <xsl:if test="n1:effectiveTime/@value!=''">
		          <tr>
			          <td class="LabelCell">
				          <xsl:text>Created On:</xsl:text>
			          </td>
			          <td class="content">
				          <xsl:apply-templates select="n1:effectiveTime" mode="Date"/>
			          </td>
		          </tr>
	          </xsl:if>
	          <xsl:for-each select="n1:telecom">
		          <tr>
			          <td class="LabelCell">
				          <xsl:apply-templates select="." mode="Label"/><xsl:text>:</xsl:text>
			          </td>
			          <td>
				          <xsl:apply-templates select="."/>
			          </td>
		          </tr>
	          </xsl:for-each>
	          <xsl:if test="n1:assignedAuthoringDevice!=''">
		          <tr>
			          <td class="LabelCell">
				          <xsl:text>System:</xsl:text>
			          </td>
			          <td>
				          <xsl:apply-templates select="n1:assignedAuthoringDevice/n1:manufacturerModelName" mode="DisplayGiven"/>
                  <xsl:apply-templates select="n1:assignedAuthoringDevice/n1:softwareName" mode="DisplayGiven"/>
			          </td>
		          </tr>
	          </xsl:if>
	          <xsl:if test="/n1:ClinicalDocument/n1:legalAuthenticator!=''">
		          <tr>
			          <td class="LabelCell">
				          <xsl:text>Signed by:</xsl:text>
			          </td>
			          <td>
				          <xsl:apply-templates select="/n1:ClinicalDocument/n1:legalAuthenticator/n1:assignedEntity/n1:assignedPerson/n1:name"/>
                  <xsl:if test="/n1:ClinicalDocument/n1:legalAuthenticator/n1:assignedEntity/n1:representedOrganization/n1:name">
                    <xsl:text> (</xsl:text>
                    <xsl:value-of select="/n1:ClinicalDocument/n1:legalAuthenticator/n1:assignedEntity/n1:representedOrganization/n1:name"/>
                    <xsl:text>)</xsl:text>
                  </xsl:if>
				          <xsl:text> on </xsl:text>
				          <xsl:apply-templates select="//n1:ClinicalDocument/n1:legalAuthenticator/n1:time" mode="Date"/>
			          </td>
		          </tr>
	          </xsl:if>
          </table>
        </td>
      </tr>
    </table>
  </div>
</div>
  
<!--Patient Information -->
<div class="PatientInfo">
  <h2><xsl:text>Patient Information</xsl:text></h2>
  <div class="PatientDetail">
     <xsl:for-each select="n1:recordTarget/n1:patientRole">
      <h3><xsl:text>Patient</xsl:text></h3>
      <table>
        <tr>
          <td>
            <table>
	            <tr>
		            <td class="LabelCell">
			            <xsl:text>Name:</xsl:text>
		            </td>
		            <td>
			            <xsl:apply-templates select="n1:patient/n1:name"/>
		            </td>
	            </tr>
	            <xsl:for-each select="n1:addr">
		            <tr>
			            <td class="LabelCell">
				            <xsl:text>Address:</xsl:text>
			            </td>
			            <td>
				            <xsl:apply-templates select="."/>
			            </td>
		            </tr>
	            </xsl:for-each>
	            <xsl:for-each select="n1:telecom">
		            <tr>
			            <td class="LabelCell">
				            <xsl:apply-templates select="." mode="Label"/><xsl:text>:</xsl:text>
			            </td>
			            <td>
				            <xsl:apply-templates select="."/>
			            </td>
		            </tr>
	            </xsl:for-each>
	            <xsl:if test="n1:patient/n1:maritalStatusCode">
		            <tr>
			            <td class="LabelCell">
				            <xsl:text>Marital Status:</xsl:text>
			            </td>
			            <td>
				            <xsl:apply-templates select="n1:patient/n1:maritalStatusCode" mode="CE"/>
			            </td>
		            </tr>
	            </xsl:if>         
	            <xsl:if test="n1:patient/n1:religiousAffiliationCode">
		            <tr>
			            <td class="LabelCell">
				            <xsl:text>Religion:</xsl:text>
			            </td>
			            <td>
				            <xsl:apply-templates select="n1:patient/n1:religiousAffiliationCode" mode="CE"/>
			            </td>
		            </tr>
	            </xsl:if>    
            </table>
          </td>
          <td>      
	          <table>
		          <xsl:if test="n1:id/@extension!=''">
			          <tr>
				          <td class="LabelCell">
					          <xsl:text>MRN:</xsl:text>
				          </td>
				          <td>
					          <xsl:value-of select="n1:id/@extension"/>
				          </td>
			          </tr>
		          </xsl:if>
		          <xsl:if test="n1:patient/n1:birthTime/@value!=''">
			          <tr>
				          <td class="LabelCell">
					          <xsl:text>Date of Birth:</xsl:text>
				          </td>
				          <td>
					          <xsl:apply-templates select="n1:patient/n1:birthTime" mode="DateNoWeekday"/>
				          </td>
			          </tr>
		          </xsl:if>
		          <xsl:if test="n1:patient/n1:administrativeGenderCode/@code!=''">
			          <tr>
				          <td class="LabelCell">
					          <xsl:text>Gender:</xsl:text>
				          </td>
				          <td>
					          <xsl:apply-templates select="n1:patient/n1:administrativeGenderCode" mode="Gender"/>
				          </td>
			          </tr>
		          </xsl:if>
		          <xsl:if test="n1:patient/n1:raceCode">
			          <tr>
				          <td class="LabelCell">
					          <xsl:text>Race:</xsl:text>
				          </td>
				          <td>
					          <xsl:apply-templates select="n1:patient/n1:raceCode" mode="CE"/>
				          </td>
			          </tr>
		          </xsl:if>
		          <xsl:if test="n1:patient/n1:ethnicGroupCode">
			          <tr>
				          <td class="LabelCell">
					          <xsl:text>Ethnicity:</xsl:text>
				          </td>
				          <td>
					          <xsl:apply-templates select="n1:patient/n1:ethnicGroupCode" mode="CE"/>
				          </td>
			          </tr>
		          </xsl:if>
		          <xsl:for-each select="n1:patient/n1:languageCommunication">
			          <tr>
				          <td class="LabelCell">
						          <xsl:text>Language:</xsl:text>
				          </td>
				          <td>
					          <xsl:apply-templates select="." mode="LanguageCommunication"/>
				          </td>
			          </tr>
		          </xsl:for-each>
	          </table>
          </td>
        </tr>
      </table>
    </xsl:for-each>
  </div>
    
  <!--Associations -->
  <xsl:for-each select="n1:participant/n1:associatedEntity">
    <div class="PatientAssociated">
      <h3>
        <xsl:choose>
          <xsl:when test="@classCode='PRS'">Personal Relationship</xsl:when>
          <xsl:when test="@classCode='GUAR'">Guardian</xsl:when>
          <xsl:when test="@classCode='NOK'">Next of Kin</xsl:when>
          <xsl:when test="@classCode='ECON'">Emergency Contact</xsl:when>
          <xsl:when test="@classCode='AGNT'">Agent</xsl:when>
          <xsl:when test="@classCode='CAREGIVER'">Caregiver</xsl:when>
          <xsl:otherwise>Other Support</xsl:otherwise>
        </xsl:choose> 
      </h3>
      <table>
        <tr>
          <td>
            <table>
	            <tr>
		            <td class="LabelCell">
			            <xsl:text>Name:</xsl:text>
		            </td>
		            <td>
			            <xsl:apply-templates select="n1:associatedPerson/n1:name"/>
		            </td>
	            </tr>
	            <xsl:for-each select="n1:addr">
		            <tr>
			            <td class="LabelCell">
				            <xsl:text>Address:</xsl:text>
			            </td>
			            <td>
				            <xsl:apply-templates select="."/>
			            </td>
		            </tr>
	            </xsl:for-each>
            </table>
          </td>
          <td>                
            <table>
              <tr>
                <td class="LabelCell">
                  <xsl:text>Relation:</xsl:text>
                </td>
                <td>
                  <xsl:apply-templates select="n1:code" mode="CE"/>
                </td>
              </tr>
	            <xsl:for-each select="n1:telecom">
		            <tr>
			            <td class="LabelCell">
				            <xsl:apply-templates select="." mode="Label"/><xsl:text>:</xsl:text>
			            </td>
			            <td>
				            <xsl:apply-templates select="."/>
			            </td>
		            </tr>
	            </xsl:for-each>
            </table>                  
          </td>
        </tr>
      </table>  
    </div>
  </xsl:for-each>
    
  <!--Providers -->
  <xsl:for-each select="n1:documentationOf/n1:serviceEvent/n1:performer">
    <div class="PatientPerformer">
      <h3>
        <xsl:choose>
          <xsl:when test="n1:functionCode"><xsl:apply-templates select="n1:functionCode" mode="CEFULL"/></xsl:when>
          <xsl:otherwise><xsl:text>Provider</xsl:text></xsl:otherwise>
        </xsl:choose>
      </h3>
      <table>
        <tr>
          <td>
            <table>
	            <tr>
		            <td class="LabelCell">
			            <xsl:text>Name:</xsl:text>
		            </td>
		            <td>
			            <xsl:apply-templates select="n1:assignedEntity/n1:assignedPerson/n1:name"/>
		            </td>
	            </tr>
              <xsl:if test="n1:assignedEntity/n1:representedOrganization/n1:name">
	              <tr>
		              <td class="LabelCell">
			              <xsl:text>Organization:</xsl:text>
		              </td>
		              <td>
			              <xsl:apply-templates select="n1:assignedEntity/n1:representedOrganization/n1:name"/>
		              </td>
	              </tr>
              </xsl:if>
	            <xsl:for-each select="n1:assignedEntity/n1:addr">
		            <tr>
			            <td class="LabelCell">
				            <xsl:text>Address:</xsl:text>
			            </td>
			            <td>
				            <xsl:apply-templates select="."/>
			            </td>
		            </tr>
	            </xsl:for-each>
              <xsl:if test="n1:assignedEntity/sdtc:patient/sdtc:id">
		            <tr>
			            <td class="LabelCell">
				            <xsl:text>MRN:</xsl:text>
			            </td>
			            <td>
				            <xsl:value-of select="n1:assignedEntity/sdtc:patient/sdtc:id/@root"/>
			            </td>
		            </tr>
              </xsl:if>
            </table>
          </td>
          <td>                
            <table>
	            <xsl:for-each select="n1:assignedEntity/n1:telecom">
		            <tr>
			            <td class="LabelCell">
				            <xsl:apply-templates select="." mode="Label"/><xsl:text>:</xsl:text>
			            </td>
			            <td>
				            <xsl:apply-templates select="."/>
			            </td>
		            </tr>
	            </xsl:for-each>
              <xsl:if test="n1:assignedEntity/n1:code">
		            <tr>
			            <td class="LabelCell">
				            <xsl:text>Type:</xsl:text>
			            </td>
			            <td>
				            <xsl:apply-templates select="n1:assignedEntity/n1:code" mode="CE"/>
			            </td>
		            </tr>
              </xsl:if>
            </table>                  
          </td>
        </tr>
      </table>
    </div>
  </xsl:for-each>
</div>
	
<!-- 
********************************************************
  CDA Body
********************************************************
--> 
<div class="StructuredBody">
  <xsl:apply-templates select="n1:component/n1:structuredBody"/>
</div>

</body>
    </html>
  </xsl:template>

  <!-- StructuredBody -->
  <xsl:template match="n1:component/n1:structuredBody">
    <h2>
      <xsl:text>Contents</xsl:text>
    </h2>
    <xsl:apply-templates select="n1:component/n1:section"/>
  </xsl:template>

  <!-- Component/Section -->
  <xsl:template match="n1:component/n1:section">
    <div class="section">
      <a href="#top" title="Top of page" class="TopLink">
        <xsl:text> [Top]</xsl:text>
      </a>
      <h3>
        <a>
          <xsl:attribute name="name">
            <xsl:value-of select="n1:title"/>
          </xsl:attribute>
        </a>
        <xsl:value-of select="n1:title"/>
      </h3>
      <div class="content">
        <xsl:apply-templates select="n1:text"/>
      </div>

      <xsl:apply-templates select="n1:component/n1:section"/>
    </div>
  </xsl:template>


  <xsl:template match="n1:name">
    <xsl:choose>
      <xsl:when test="n1:given or n1:family">
        <xsl:apply-templates select="n1:prefix" mode="DisplayGiven"/>
        <xsl:apply-templates select="n1:given" mode="DisplayGiven"/>
        <xsl:apply-templates select="n1:family" mode="DisplayGiven"/>
        <xsl:apply-templates select="n1:suffix" mode="DisplayGiven"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="."/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="n1:addr">
    <xsl:choose>
      <xsl:when test="count(*)=0">
        <xsl:value-of select="."/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:if test="n1:careOf!=''">
          <xsl:value-of select="n1:careOf"/>
          <br/>
        </xsl:if>
        <xsl:for-each select="n1:streetAddressLine">
          <xsl:value-of select="."/>
          <br/>
        </xsl:for-each>
        <xsl:if test="n1:houseNumber!=''">
          <xsl:apply-templates select="n1:houseNumber" mode="DisplayGiven"/>
          <xsl:apply-templates select="n1:direction" mode="DisplayGiven"/>
          <xsl:apply-templates select="n1:streetName" mode="DisplayGiven"/>
          <br/>
        </xsl:if>
        <xsl:if test="n1:city!=''">
          <xsl:value-of select="n1:city"/>, <xsl:apply-templates select="n1:state" mode="DisplayGiven"/><xsl:apply-templates select="n1:postalCode" mode="DisplayGiven"/><br/>
        </xsl:if>
        <xsl:if test="n1:country!=''">
          <xsl:value-of select="n1:country"/>
          <br/>
        </xsl:if>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*" mode="DateNoWeekday">
    <xsl:choose>
      <!-- a range of dates -->
      <xsl:when test="n1:low or n1:high">
        <xsl:choose>
          <xsl:when test="n1:low/@value='0'">
            <xsl:apply-templates select="n1:high" mode="DateNoWeekday"/>
          </xsl:when>
          <xsl:when test="n1:high/@value='0'">
            <xsl:apply-templates select="n1:low" mode="DateNoWeekday"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:apply-templates select="n1:low" mode="DateNoWeekday"/>
            <xsl:text> - </xsl:text>
            <xsl:apply-templates select="n1:high" mode="DateNoWeekday"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <!-- date only -->
      <xsl:when test="string-length(@value)=8">
        <xsl:apply-templates select="." mode="DateDisplayNoWeekday"/>
      </xsl:when>
      <!-- date + time (may include time zone which is not processed) -->
      <xsl:when test="string-length(@value)&gt;11">
        <xsl:apply-templates select="." mode="DateDisplayNoWeekday"/>
        <xsl:text> </xsl:text>

        <xsl:apply-templates select="." mode="TimeDisplay"/>
        <xsl:if test="substring(@value, 15, 1)='-' or substring(@value, 15, 1)='+'">
          <xsl:apply-templates select="." mode="TimeZoneDisplay"/>
        </xsl:if>

      </xsl:when>
      <!-- year only -->
      <xsl:when test="string-length(@value)=4">
        <xsl:value-of select="@value"/>
      </xsl:when>
      <!-- explicitly unknown -->
      <xsl:when test="@value='0'">
        <xsl:text>Unspecified</xsl:text>
      </xsl:when>
      <!-- unknown format -->
      <xsl:otherwise>
        <xsl:value-of select="."/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*" mode="Date">
    <xsl:choose>
      <!-- a range of dates -->
      <xsl:when test="n1:low or n1:high">
        <xsl:choose>
          <xsl:when test="n1:low/@value='0'">
            <xsl:apply-templates select="n1:high" mode="Date"/>
          </xsl:when>
          <xsl:when test="n1:high/@value='0'">
            <xsl:apply-templates select="n1:low" mode="Date"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:apply-templates select="n1:low" mode="Date"/>
            <xsl:text> - </xsl:text>
            <xsl:apply-templates select="n1:high" mode="Date"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <!-- date only -->
      <xsl:when test="string-length(@value)=8">
        <xsl:apply-templates select="." mode="DateDisplay"/>
      </xsl:when>
      <!-- date + time (may include time zone which is not processed) -->
      <xsl:when test="string-length(@value)&gt;11">
        <xsl:apply-templates select="." mode="DateDisplay"/>
        <xsl:text> </xsl:text>
        <xsl:apply-templates select="." mode="TimeDisplay"/>
        <xsl:if test="substring(@value, 15, 1)='-' or substring(@value, 15, 1)='+'">
          <xsl:apply-templates select="." mode="TimeZoneDisplay"/>
        </xsl:if>
      </xsl:when>
      <!-- year only -->
      <xsl:when test="string-length(@value)=4">
        <xsl:value-of select="@value"/>
      </xsl:when>
      <!-- explicitly unknown -->
      <xsl:when test="@value='0'">
        <xsl:text>Unspecified</xsl:text>
      </xsl:when>
      <!-- unknown format -->
      <xsl:otherwise>
        <xsl:value-of select="."/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*" mode="DateDisplay">
    <xsl:apply-templates select="." mode="DayOfWeekDisplay"/>
    <xsl:text>, </xsl:text>
    <xsl:variable name="month" select="substring(@value, 5, 2)"/>
    <xsl:choose>
      <xsl:when test="$month='01'">
        <xsl:text>January </xsl:text>
      </xsl:when>
      <xsl:when test="$month='02'">
        <xsl:text>February </xsl:text>
      </xsl:when>
      <xsl:when test="$month='03'">
        <xsl:text>March </xsl:text>
      </xsl:when>
      <xsl:when test="$month='04'">
        <xsl:text>April </xsl:text>
      </xsl:when>
      <xsl:when test="$month='05'">
        <xsl:text>May </xsl:text>
      </xsl:when>
      <xsl:when test="$month='06'">
        <xsl:text>June </xsl:text>
      </xsl:when>
      <xsl:when test="$month='07'">
        <xsl:text>July </xsl:text>
      </xsl:when>
      <xsl:when test="$month='08'">
        <xsl:text>August </xsl:text>
      </xsl:when>
      <xsl:when test="$month='09'">
        <xsl:text>September </xsl:text>
      </xsl:when>
      <xsl:when test="$month='10'">
        <xsl:text>October </xsl:text>
      </xsl:when>
      <xsl:when test="$month='11'">
        <xsl:text>November </xsl:text>
      </xsl:when>
      <xsl:when test="$month='12'">
        <xsl:text>December </xsl:text>
      </xsl:when>
    </xsl:choose>
    <xsl:choose>
      <xsl:when test='substring(@value, 7, 1)="0"'>
        <xsl:value-of select="substring(@value, 8, 1)"/>
        <xsl:text>, </xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="substring(@value, 7, 2)"/>
        <xsl:text>, </xsl:text>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:value-of select="substring(@value, 1, 4)"/>
  </xsl:template>

  <xsl:template match="*" mode="DateDisplayNoWeekday">
    <xsl:variable name="month" select="substring(@value, 5, 2)"/>
    <xsl:choose>
      <xsl:when test="$month='01'">
        <xsl:text>January </xsl:text>
      </xsl:when>
      <xsl:when test="$month='02'">
        <xsl:text>February </xsl:text>
      </xsl:when>
      <xsl:when test="$month='03'">
        <xsl:text>March </xsl:text>
      </xsl:when>
      <xsl:when test="$month='04'">
        <xsl:text>April </xsl:text>
      </xsl:when>
      <xsl:when test="$month='05'">
        <xsl:text>May </xsl:text>
      </xsl:when>
      <xsl:when test="$month='06'">
        <xsl:text>June </xsl:text>
      </xsl:when>
      <xsl:when test="$month='07'">
        <xsl:text>July </xsl:text>
      </xsl:when>
      <xsl:when test="$month='08'">
        <xsl:text>August </xsl:text>
      </xsl:when>
      <xsl:when test="$month='09'">
        <xsl:text>September </xsl:text>
      </xsl:when>
      <xsl:when test="$month='10'">
        <xsl:text>October </xsl:text>
      </xsl:when>
      <xsl:when test="$month='11'">
        <xsl:text>November </xsl:text>
      </xsl:when>
      <xsl:when test="$month='12'">
        <xsl:text>December </xsl:text>
      </xsl:when>
    </xsl:choose>
    <xsl:choose>
      <xsl:when test='substring(@value, 7, 1)="0"'>
        <xsl:value-of select="substring(@value, 8, 1)"/>
        <xsl:text>, </xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="substring(@value, 7, 2)"/>
        <xsl:text>, </xsl:text>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:value-of select="substring(@value, 1, 4)"/>
  </xsl:template>

  <xsl:template match="*" mode="DayOfWeekDisplay">
    <!-- Using Zeller's Rule -->
    <xsl:variable name="Month">
      <xsl:choose>
        <xsl:when test="substring(@value, 5, 2)&lt;3">
          <xsl:value-of select="substring(@value, 5, 2) + 10"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="substring(@value, 5, 2) - 2"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="Century">
      <xsl:choose>
        <xsl:when test="$Month&gt;10">
          <xsl:value-of select="substring(substring(@value, 1, 4) - 1, 1, 2)"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="substring(@value, 1, 2)"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="Year">
      <xsl:choose>
        <xsl:when test="$Month&gt;10">
          <xsl:value-of select="substring(substring(@value, 1, 4) - 1, 3, 2)"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="substring(@value, 3, 2)"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="Day" select="substring(@value, 7, 2)"/>

    <xsl:variable name="DayOfWeek">
      <xsl:choose>
        <xsl:when test="($Day + floor((13 * $Month - 1) div 5) + $Year + floor($Year div 4) + floor($Century div 4) - (2 * $Century)) mod 7&lt;0">
          <xsl:value-of select="($Day + floor((13 * $Month - 1) div 5) + $Year + floor($Year div 4) + floor($Century div 4) - (2 * $Century)) mod 7+7"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="($Day + floor((13 * $Month - 1) div 5) + $Year + floor($Year div 4) + floor($Century div 4) - (2 * $Century)) mod 7"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="$DayOfWeek=0">Sunday</xsl:when>
      <xsl:when test="$DayOfWeek=1">Monday</xsl:when>
      <xsl:when test="$DayOfWeek=2">Tuesday</xsl:when>
      <xsl:when test="$DayOfWeek=3">Wednesday</xsl:when>
      <xsl:when test="$DayOfWeek=4">Thursday</xsl:when>
      <xsl:when test="$DayOfWeek=5">Friday</xsl:when>
      <xsl:when test="$DayOfWeek=6">Saturday</xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*" mode="TimeDisplay">

    <xsl:variable name="Hours">
      <xsl:choose>
        <xsl:when test="substring(@value, 9, 2)=0">12</xsl:when>
        <xsl:when test="substring(@value, 9, 2)&gt;12">
          <xsl:value-of select="substring(@value, 9, 2)-12"/>
        </xsl:when>
        <xsl:when test="substring(@value, 9, 1)=0">
          <xsl:value-of select="substring(@value, 10, 1)"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="substring(@value, 9, 2)"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="Meridian">
      <xsl:choose>
        <xsl:when test="substring(@value, 9, 2)&gt;11"> pm</xsl:when>
        <xsl:otherwise> am</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="Display">
      <xsl:value-of select="$Hours"/>
      <xsl:text>:</xsl:text>
      <xsl:value-of select="substring(@value, 11, 2)"/>
      <xsl:if test="string-length(@value)&gt;5">
        <xsl:text>:</xsl:text>
        <xsl:value-of select="substring(@value, 13, 2)"/>
      </xsl:if>
    </xsl:variable>

    <xsl:if test="$Display!='12:00:00' and $Display!='12:00'">
      <xsl:text>at </xsl:text>
      <xsl:value-of select="$Display"/>
      <xsl:value-of select="$Meridian"/>
    </xsl:if>
  </xsl:template>

  <xsl:template match="*" mode="TimeZoneDisplay">
    <xsl:if test="substring(@value, 9, 6) != '000000'">
      <xsl:text> </xsl:text>
      <small>
        <i>
          (<xsl:value-of select="substring(@value, 15, string-length(@value) - 14)"/>)
        </i>
      </small>
    </xsl:if>
  </xsl:template>

  <xsl:template match="n1:telecom">
    <xsl:choose>
      <xsl:when test="substring(@value, 4, 1)=':'">
        <xsl:value-of select="substring(@value, 5, string-length(@value) - 4)"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="@value"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="n1:telecom" mode="Label">
    <xsl:choose>
      <xsl:when test="@use">
        <xsl:variable name="Postfix">
          <xsl:choose>
            <xsl:when test="substring(@value, 1, 3)='tel' and @use!='PG'">Telephone</xsl:when>
            <xsl:when test="substring(@value, 1, 3)='fax'">Fax</xsl:when>
          </xsl:choose>
        </xsl:variable>
        <xsl:choose>
          <xsl:when test="@use='H'">Home</xsl:when>
          <xsl:when test="@use='HP'">Home</xsl:when>
          <xsl:when test="@use='HV'">Vacation Home</xsl:when>
          <xsl:when test="@use='WP'">Work</xsl:when>
          <xsl:when test="@use='DIR'">Direct</xsl:when>
          <xsl:when test="@use='BAD'">Bad</xsl:when>
          <xsl:when test="@use='TMP'">Temporary</xsl:when>
          <xsl:when test="@use='AS'">Answering Service</xsl:when>
          <xsl:when test="@use='EC'">Emergency</xsl:when>
          <xsl:when test="@use='MC'">Mobile</xsl:when>
          <xsl:when test="@use='PG'">Pager</xsl:when>
        </xsl:choose>
        <xsl:text> </xsl:text>
        <xsl:value-of select="$Postfix"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>Other</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*" mode="CE">
    <xsl:choose>
      <xsl:when test="n1:originalText!=''">
        <xsl:value-of select="n1:originalText"/>
      </xsl:when>
      <xsl:when test="@code!='' or @displayName!=''">
        <xsl:choose>
          <xsl:when test="@displayName!=''">
            <xsl:value-of select="@displayName"/>
            <xsl:text> </xsl:text>
            <xsl:if test="@code!=''">
              <small>
                <i>
                  <xsl:text>(</xsl:text>
                  <xsl:apply-templates select="@code" mode="DisplayGiven"/>
                  <xsl:text> </xsl:text>
                  <xsl:value-of select="@codeSystemName"/>
                  <xsl:text>)</xsl:text>
                </i>
              </small>
            </xsl:if>
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="@code"/>
            <xsl:value-of select="@codeSystemName"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="."/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>


  <xsl:template match="n1:functionCode" mode="CEFULL">
    <xsl:choose>
      <xsl:when test="@displayName!=''">
        <xsl:value-of select="@displayName"/>
        <xsl:text> </xsl:text>

        <xsl:if test="@code!=''">
          <small>
            <i>
              <xsl:text>(</xsl:text>
              <xsl:apply-templates select="@code" mode="DisplayGiven"/>
              <xsl:text> </xsl:text>
              <xsl:value-of select="@codeSystemName"/>
              <xsl:text>)</xsl:text>
            </i>
          </small>
        </xsl:if>
      </xsl:when>
      <xsl:when test="@code='RP'">
        <xsl:text>Referring Provider</xsl:text>        
      </xsl:when>
      <xsl:when test="@code='PP'">
        <xsl:text>Primary Provider</xsl:text>
      </xsl:when>
      <xsl:when test="@code='CP'">
        <xsl:text>Consulting Provider</xsl:text>
      </xsl:when>
    </xsl:choose>
    
    <xsl:if test="n1:originalText!=''">
      <xsl:text> - </xsl:text>
      <xsl:value-of select="n1:originalText"/>
    </xsl:if>
  </xsl:template>
  
  <xsl:template match="*" mode="CEFULL">
    <xsl:choose>
      <xsl:when test="@code!='' or @displayName!=''">
        <xsl:choose>
          <xsl:when test="@displayName!=''">

            <xsl:value-of select="@displayName"/>
            <xsl:text> </xsl:text>

            <xsl:if test="@code!=''">
              <small>
                <i>
                  <xsl:text>(</xsl:text>
                  <xsl:apply-templates select="@code" mode="DisplayGiven"/>
                  <xsl:text> </xsl:text>
                  <xsl:value-of select="@codeSystemName"/>
                  <xsl:text>)</xsl:text>
                </i>
              </small>
            </xsl:if>
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="@code"/>
            <xsl:value-of select="@codeSystemName"/>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:if test="n1:originalText!=''">
          <xsl:text> - </xsl:text>
          <xsl:value-of select="n1:originalText"/>
        </xsl:if>
      </xsl:when>
      <xsl:when test="n1:originalText!=''">
        <xsl:value-of select="n1:originalText"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="."/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*" mode="Participant">
    <xsl:if test="n1:code">
      <xsl:apply-templates select="n1:code" mode="CE"/>
      <br/>
    </xsl:if>
    <xsl:if test="n1:associatedPerson/n1:name">
      <xsl:apply-templates select="n1:associatedPerson/n1:name"/>
      <br/>
    </xsl:if>
    <xsl:if test="n1:addr">
      <xsl:apply-templates select="n1:addr"/>
    </xsl:if>
    <xsl:if test="n1:telecom">
      <xsl:apply-templates select="n1:telecom" mode="Label"/>
      <xsl:text>: </xsl:text>
      <xsl:apply-templates select="n1:telecom"/>
      <br/>
    </xsl:if>
  </xsl:template>

  <xsl:template match="*" mode="Gender">
    <xsl:choose>
      <xsl:when test="@code='M'">Male</xsl:when>
      <xsl:when test="@code='F'">Female</xsl:when>
      <xsl:when test="@code='UN'">Undifferentiated</xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="@code"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>


  <xsl:template match="*" mode="LanguageCommunication">
    <xsl:choose>
      <xsl:when test="n1:languageCode">
        <xsl:apply-templates select="n1:languageCode" mode="ISO639LanguageISO3166Country"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>Unspecified</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:if test="n1:preferenceInd/@value='true' or n1:modeCode">
      <xsl:text> (</xsl:text>
      <xsl:if test="n1:preferenceInd/@value='true'">
        <xsl:text>Preferred</xsl:text>
        <xsl:if test="n1:modeCode">
          <xsl:text>, </xsl:text>
        </xsl:if>
      </xsl:if>
      <xsl:if test="n1:modeCode">
        <xsl:apply-templates select="n1:modeCode" mode="CE"/>
      </xsl:if>
      <xsl:text>)</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template match="*" mode="componentEntry"/>

  <xsl:template match="*" mode="DisplayGiven">
    <xsl:if test=".">
      <xsl:value-of select="."/>
      <xsl:text> </xsl:text>
    </xsl:if>
  </xsl:template>


  <!-- Component text rendering -->
  <!--   Text   -->
  <xsl:template match="n1:text" mode="componentText">
    <xsl:apply-templates/>
    <br/>
  </xsl:template>

  <xsl:template match="n1:linkHtml">
    <a>
      <xsl:attribute name="href">
        <xsl:value-of select="@href"/>
      </xsl:attribute>

      <xsl:apply-templates/>
    </a>
  </xsl:template>

  <!--      Tables   -->
  <xsl:template match="n1:table/@*|n1:thead/@*|n1:tfoot/@*|n1:tbody/@*|n1:colgroup/@*|n1:col/@*|n1:tr/@*|n1:th/@*|n1:td/@*">
    <xsl:copy>
      <xsl:apply-templates/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="n1:table">
    <table>
      <xsl:apply-templates/>
    </table>
  </xsl:template>

  <xsl:template match="n1:thead">
    <thead>
      <xsl:apply-templates/>
    </thead>
  </xsl:template>

  <xsl:template match="n1:tfoot">
    <tfoot>
      <xsl:apply-templates/>
    </tfoot>
  </xsl:template>

  <xsl:template match="n1:tbody">
    <tbody>
      <xsl:apply-templates/>
    </tbody>
  </xsl:template>

  <xsl:template match="n1:colgroup">
    <colgroup>
      <xsl:apply-templates/>
    </colgroup>
  </xsl:template>

  <xsl:template match="n1:col">
    <col>
      <xsl:apply-templates/>
    </col>
  </xsl:template>

  <xsl:template match="n1:tr">
    <!--
	<xsl:variable name="Class">
		<xsl:if test="name(parent::node())!='thead'"><xsl:choose><xsl:when test="position() mod 2 = 0">first</xsl:when><xsl:otherwise>second</xsl:otherwise></xsl:choose></xsl:if>
	</xsl:variable>
	<tr class="{$Class}">	
		<xsl:apply-templates/>
	</tr>
-->
    <tr>
      <xsl:apply-templates/>
    </tr>
  </xsl:template>

  <xsl:template match="n1:th">
    <th>
      <xsl:apply-templates/>
    </th>
  </xsl:template>

  <xsl:template match="n1:td">
    <td>
      <xsl:apply-templates/>
    </td>
  </xsl:template>

  <xsl:template match="n1:table/n1:caption">
    <span style="font-weight:bold;">
      <xsl:apply-templates/>
    </span>
  </xsl:template>



  <!--   paragraph  -->
  <xsl:template match="n1:paragraph">
    <p>
      <xsl:apply-templates/>
    </p>
  </xsl:template>

  <!--     Content w/ deleted text is hidden -->
  <xsl:template match="n1:content[@revised='delete']"/>

  <!--   content  -->
  <xsl:template match="n1:content">
    <xsl:apply-templates/>
  </xsl:template>

  <!--   list  -->
  <xsl:template match="n1:list">
    <xsl:if test="n1:caption">
      <span style="font-weight:bold;">
        <xsl:apply-templates select="n1:caption"/>
      </span>
    </xsl:if>
    <ul>
      <xsl:for-each select="n1:item">
        <li>
          <xsl:apply-templates/>
        </li>
      </xsl:for-each>
    </ul>
  </xsl:template>

  <xsl:template match="n1:list[@listType='ordered']">
    <xsl:if test="n1:caption">
      <span style="font-weight:bold;">
        <xsl:apply-templates select="n1:caption"/>
      </span>
    </xsl:if>
    <ol>
      <xsl:for-each select="n1:item">
        <li>
          <xsl:apply-templates/>
        </li>
      </xsl:for-each>
    </ol>
  </xsl:template>

  <!--   caption  -->
  <xsl:template match="n1:caption">
    <xsl:apply-templates/>
    <xsl:text>: </xsl:text>
  </xsl:template>



  <!--   RenderMultiMedia 

this currently only handles GIF's and JPEG's.  It could, however,
be extended by including other image MIME types in the predicate
and/or by generating <object> or <applet> tag with the correct
params depending on the media type  @ID  =$imageRef     referencedObject
-->
  <xsl:template match="n1:renderMultiMedia">
    <xsl:variable name="imageRef" select="@referencedObject"/>
    <xsl:choose>
      <xsl:when test="//n1:regionOfInterest[@ID=$imageRef]">
        <!-- Here is where the Region of Interest image referencing goes -->
        <xsl:if test='//n1:regionOfInterest[@ID=$imageRef]//n1:observationMedia/n1:value[@mediaType="image/gif" or @mediaType="image/jpeg"]'>
          <br clear='all'/>
          <xsl:element name='img'>
            <xsl:attribute name='src'>
              graphics/
              <xsl:value-of select='//n1:regionOfInterest[@ID=$imageRef]//n1:observationMedia/n1:value/n1:reference/@value'/>
            </xsl:attribute>
          </xsl:element>
        </xsl:if>
      </xsl:when>
      <xsl:otherwise>
        <!-- Here is where the direct MultiMedia image referencing goes -->
        <xsl:if test='//n1:observationMedia[@ID=$imageRef]/n1:value[@mediaType="image/gif" or @mediaType="image/jpeg"]'>
          <br clear='all'/>
          <xsl:element name='img'>
            <xsl:attribute name='src'>
              graphics/
              <xsl:value-of select='//n1:observationMedia[@ID=$imageRef]/n1:value/n1:reference/@value'/>
            </xsl:attribute>
          </xsl:element>
        </xsl:if>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- 	Stylecode processing   
Supports Bold, Underline and Italics display

-->

  <xsl:template match="//n1:*[@styleCode]">
    <xsl:if test="@styleCode='Bold'">
      <xsl:element name='b'>
        <xsl:apply-templates/>
      </xsl:element>
    </xsl:if>

    <xsl:if test="@styleCode='Italics'">
      <xsl:element name='i'>
        <xsl:apply-templates/>
      </xsl:element>
    </xsl:if>

    <xsl:if test="@styleCode='Underline'">
      <xsl:element name='u'>
        <xsl:apply-templates/>
      </xsl:element>
    </xsl:if>

    <xsl:if test="contains(@styleCode,'Bold') and contains(@styleCode,'Italics') and not (contains(@styleCode, 'Underline'))">
      <xsl:element name='b'>
        <xsl:element name='i'>
          <xsl:apply-templates/>
        </xsl:element>
      </xsl:element>
    </xsl:if>

    <xsl:if test="contains(@styleCode,'Bold') and contains(@styleCode,'Underline') and not (contains(@styleCode, 'Italics'))">
      <xsl:element name='b'>
        <xsl:element name='u'>
          <xsl:apply-templates/>
        </xsl:element>
      </xsl:element>
    </xsl:if>

    <xsl:if test="contains(@styleCode,'Italics') and contains(@styleCode,'Underline') and not (contains(@styleCode, 'Bold'))">
      <xsl:element name='i'>
        <xsl:element name='u'>
          <xsl:apply-templates/>
        </xsl:element>
      </xsl:element>
    </xsl:if>

    <xsl:if test="contains(@styleCode,'Italics') and contains(@styleCode,'Underline') and contains(@styleCode, 'Bold')">
      <xsl:element name='b'>
        <xsl:element name='i'>
          <xsl:element name='u'>
            <xsl:apply-templates/>
          </xsl:element>
        </xsl:element>
      </xsl:element>
    </xsl:if>

  </xsl:template>

  <!-- 	Superscript or Subscript   -->
  <xsl:template match="n1:sup">
    <xsl:element name='sup'>
      <xsl:apply-templates/>
    </xsl:element>
  </xsl:template>

  <xsl:template match="n1:sub">
    <xsl:element name='sub'>
      <xsl:apply-templates/>
    </xsl:element>
  </xsl:template>

  <xsl:template match="n1:languageCode" mode="ISO639LanguageISO3166Country">
    <xsl:variable name="languageCode" select="substring(@code, 1, 2)"/>
    <xsl:variable name="countryCode" select="substring(@code, 4, 2)"/>

    <xsl:variable name="languageName">
      <xsl:choose>
        <xsl:when test="$languageCode='aa'">Afar</xsl:when>
        <xsl:when test="$languageCode='ab'">Abkhazian</xsl:when>
        <xsl:when test="$languageCode='af'">Afrikaans</xsl:when>
        <xsl:when test="$languageCode='ak'">Akan</xsl:when>
        <xsl:when test="$languageCode='sq'">Albanian</xsl:when>
        <xsl:when test="$languageCode='am'">Amharic</xsl:when>
        <xsl:when test="$languageCode='ar'">Arabic</xsl:when>
        <xsl:when test="$languageCode='an'">Aragonese</xsl:when>
        <xsl:when test="$languageCode='hy'">Armenian</xsl:when>
        <xsl:when test="$languageCode='as'">Assamese</xsl:when>
        <xsl:when test="$languageCode='av'">Avaric</xsl:when>
        <xsl:when test="$languageCode='ae'">Avestan</xsl:when>
        <xsl:when test="$languageCode='ay'">Aymara</xsl:when>
        <xsl:when test="$languageCode='az'">Azerbaijani</xsl:when>
        <xsl:when test="$languageCode='ba'">Bashkir</xsl:when>
        <xsl:when test="$languageCode='bm'">Bambara</xsl:when>
        <xsl:when test="$languageCode='eu'">Basque</xsl:when>
        <xsl:when test="$languageCode='be'">Belarusian</xsl:when>
        <xsl:when test="$languageCode='bn'">Bengali</xsl:when>
        <xsl:when test="$languageCode='bh'">Bihari</xsl:when>
        <xsl:when test="$languageCode='bi'">Bislama</xsl:when>
        <xsl:when test="$languageCode='bs'">Bosnian</xsl:when>
        <xsl:when test="$languageCode='br'">Breton</xsl:when>
        <xsl:when test="$languageCode='bg'">Bulgarian</xsl:when>
        <xsl:when test="$languageCode='my'">Burmese</xsl:when>
        <xsl:when test="$languageCode='ca'">Catalan; Valencian</xsl:when>
        <xsl:when test="$languageCode='ch'">Chamorro</xsl:when>
        <xsl:when test="$languageCode='ce'">Chechen</xsl:when>
        <xsl:when test="$languageCode='zh'">Chinese</xsl:when>
        <xsl:when test="$languageCode='cu'">Church Slavic; Old Slavonic</xsl:when>
        <xsl:when test="$languageCode='cv'">Chuvash</xsl:when>
        <xsl:when test="$languageCode='kw'">Cornish</xsl:when>
        <xsl:when test="$languageCode='co'">Corsican</xsl:when>
        <xsl:when test="$languageCode='cr'">Cree</xsl:when>
        <xsl:when test="$languageCode='cs'">Czech</xsl:when>
        <xsl:when test="$languageCode='da'">Danish</xsl:when>
        <xsl:when test="$languageCode='dv'">Divehi</xsl:when>
        <xsl:when test="$languageCode='nl'">Dutch</xsl:when>
        <xsl:when test="$languageCode='dz'">Dzongkha</xsl:when>
        <xsl:when test="$languageCode='en'">English</xsl:when>
        <xsl:when test="$languageCode='eo'">Esperanto</xsl:when>
        <xsl:when test="$languageCode='et'">Estonian</xsl:when>
        <xsl:when test="$languageCode='ee'">Ewe</xsl:when>
        <xsl:when test="$languageCode='fo'">Faroese</xsl:when>
        <xsl:when test="$languageCode='fj'">Fijian</xsl:when>
        <xsl:when test="$languageCode='fi'">Finnish</xsl:when>
        <xsl:when test="$languageCode='fr'">French</xsl:when>
        <xsl:when test="$languageCode='fy'">Western Frisian</xsl:when>
        <xsl:when test="$languageCode='ff'">Fulah</xsl:when>
        <xsl:when test="$languageCode='ka'">Georgian</xsl:when>
        <xsl:when test="$languageCode='de'">German</xsl:when>
        <xsl:when test="$languageCode='gd'">Gaelic; Scottish Gaelic</xsl:when>
        <xsl:when test="$languageCode='ga'">Irish</xsl:when>
        <xsl:when test="$languageCode='gl'">Galician</xsl:when>
        <xsl:when test="$languageCode='gv'">Manx</xsl:when>
        <xsl:when test="$languageCode='el'">Greek; Modern</xsl:when>
        <xsl:when test="$languageCode='gn'">Guarani</xsl:when>
        <xsl:when test="$languageCode='gu'">Gujarati</xsl:when>
        <xsl:when test="$languageCode='ht'">Haitian; Haitian Creole</xsl:when>
        <xsl:when test="$languageCode='ha'">Hausa</xsl:when>
        <xsl:when test="$languageCode='he'">Hebrew</xsl:when>
        <xsl:when test="$languageCode='hz'">Herero</xsl:when>
        <xsl:when test="$languageCode='hi'">Hindi</xsl:when>
        <xsl:when test="$languageCode='ho'">Hiri Motu</xsl:when>
        <xsl:when test="$languageCode='hr'">Croatian</xsl:when>
        <xsl:when test="$languageCode='hu'">Hungarian</xsl:when>
        <xsl:when test="$languageCode='ig'">Igbo</xsl:when>
        <xsl:when test="$languageCode='is'">Icelandic</xsl:when>
        <xsl:when test="$languageCode='io'">Ido</xsl:when>
        <xsl:when test="$languageCode='ii'">Sichuan Yi; Nuosu</xsl:when>
        <xsl:when test="$languageCode='iu'">Inuktitut</xsl:when>
        <xsl:when test="$languageCode='ie'">Interlingue; Occidental</xsl:when>
        <xsl:when test="$languageCode='ia'">Interlingua</xsl:when>
        <xsl:when test="$languageCode='id'">Indonesian</xsl:when>
        <xsl:when test="$languageCode='ik'">Inupiaq</xsl:when>
        <xsl:when test="$languageCode='it'">Italian</xsl:when>
        <xsl:when test="$languageCode='jv'">Javanese</xsl:when>
        <xsl:when test="$languageCode='ja'">Japanese</xsl:when>
        <xsl:when test="$languageCode='kl'">Kalaallisut; Greenlandic</xsl:when>
        <xsl:when test="$languageCode='kn'">Kannada</xsl:when>
        <xsl:when test="$languageCode='ks'">Kashmiri</xsl:when>
        <xsl:when test="$languageCode='kr'">Kanuri</xsl:when>
        <xsl:when test="$languageCode='kk'">Kazakh</xsl:when>
        <xsl:when test="$languageCode='km'">Central Khmer</xsl:when>
        <xsl:when test="$languageCode='ki'">Kikuyu</xsl:when>
        <xsl:when test="$languageCode='rw'">Kinyarwanda</xsl:when>
        <xsl:when test="$languageCode='ky'">Kirghiz</xsl:when>
        <xsl:when test="$languageCode='kv'">Komi</xsl:when>
        <xsl:when test="$languageCode='kg'">Kongo</xsl:when>
        <xsl:when test="$languageCode='ko'">Korean</xsl:when>
        <xsl:when test="$languageCode='kj'">Kuanyama</xsl:when>
        <xsl:when test="$languageCode='ku'">Kurdish</xsl:when>
        <xsl:when test="$languageCode='lo'">Lao</xsl:when>
        <xsl:when test="$languageCode='la'">Latin</xsl:when>
        <xsl:when test="$languageCode='lv'">Latvian</xsl:when>
        <xsl:when test="$languageCode='li'">Limburgan</xsl:when>
        <xsl:when test="$languageCode='ln'">Lingala</xsl:when>
        <xsl:when test="$languageCode='lt'">Lithuanian</xsl:when>
        <xsl:when test="$languageCode='lb'">Luxembourgish</xsl:when>
        <xsl:when test="$languageCode='lu'">Luba-Katanga</xsl:when>
        <xsl:when test="$languageCode='lg'">Ganda</xsl:when>
        <xsl:when test="$languageCode='mk'">Macedonian</xsl:when>
        <xsl:when test="$languageCode='mh'">Marshallese</xsl:when>
        <xsl:when test="$languageCode='ml'">Malayalam</xsl:when>
        <xsl:when test="$languageCode='mi'">Maori</xsl:when>
        <xsl:when test="$languageCode='mr'">Marathi</xsl:when>
        <xsl:when test="$languageCode='ms'">Malay</xsl:when>
        <xsl:when test="$languageCode='mg'">Malagasy</xsl:when>
        <xsl:when test="$languageCode='mt'">Maltese</xsl:when>
        <xsl:when test="$languageCode='mn'">Mongolian</xsl:when>
        <xsl:when test="$languageCode='na'">Nauru</xsl:when>
        <xsl:when test="$languageCode='nv'">Navajo</xsl:when>
        <xsl:when test="$languageCode='nr'">South Ndebele</xsl:when>
        <xsl:when test="$languageCode='nd'">North Ndebele</xsl:when>
        <xsl:when test="$languageCode='ng'">Ndonga</xsl:when>
        <xsl:when test="$languageCode='ne'">Nepali</xsl:when>
        <xsl:when test="$languageCode='nn'">Norwegian Nynorsk</xsl:when>
        <xsl:when test="$languageCode='nb'">Norwegian Bokmål</xsl:when>
        <xsl:when test="$languageCode='no'">Norwegian</xsl:when>
        <xsl:when test="$languageCode='ny'">Chichewa</xsl:when>
        <xsl:when test="$languageCode='oc'">Occitan</xsl:when>
        <xsl:when test="$languageCode='oj'">Ojibwa</xsl:when>
        <xsl:when test="$languageCode='or'">Oriya</xsl:when>
        <xsl:when test="$languageCode='om'">Oromo</xsl:when>
        <xsl:when test="$languageCode='os'">Ossetian</xsl:when>
        <xsl:when test="$languageCode='pa'">Panjabi</xsl:when>
        <xsl:when test="$languageCode='fa'">Persian</xsl:when>
        <xsl:when test="$languageCode='pi'">Pali</xsl:when>
        <xsl:when test="$languageCode='pl'">Polish</xsl:when>
        <xsl:when test="$languageCode='pt'">Portuguese</xsl:when>
        <xsl:when test="$languageCode='ps'">Pushto</xsl:when>
        <xsl:when test="$languageCode='qu'">Quechua</xsl:when>
        <xsl:when test="$languageCode='rm'">Romansh</xsl:when>
        <xsl:when test="$languageCode='ro'">Romanian</xsl:when>
        <xsl:when test="$languageCode='rn'">Rundi</xsl:when>
        <xsl:when test="$languageCode='ru'">Russian</xsl:when>
        <xsl:when test="$languageCode='sg'">Sango</xsl:when>
        <xsl:when test="$languageCode='sa'">Sanskrit</xsl:when>
        <xsl:when test="$languageCode='si'">Sinhala</xsl:when>
        <xsl:when test="$languageCode='sk'">Slovak</xsl:when>
        <xsl:when test="$languageCode='sl'">Slovenian</xsl:when>
        <xsl:when test="$languageCode='se'">Northern Sami</xsl:when>
        <xsl:when test="$languageCode='sm'">Samoan</xsl:when>
        <xsl:when test="$languageCode='sn'">Shona</xsl:when>
        <xsl:when test="$languageCode='sd'">Sindhi</xsl:when>
        <xsl:when test="$languageCode='so'">Somali</xsl:when>
        <xsl:when test="$languageCode='st'">Sotho</xsl:when>
        <xsl:when test="$languageCode='es'">Spanish</xsl:when>
        <xsl:when test="$languageCode='sc'">Sardinian</xsl:when>
        <xsl:when test="$languageCode='sr'">Serbian</xsl:when>
        <xsl:when test="$languageCode='ss'">Swati</xsl:when>
        <xsl:when test="$languageCode='su'">Sundanese</xsl:when>
        <xsl:when test="$languageCode='sw'">Swahili</xsl:when>
        <xsl:when test="$languageCode='sv'">Swedish</xsl:when>
        <xsl:when test="$languageCode='ty'">Tahitian</xsl:when>
        <xsl:when test="$languageCode='ta'">Tamil</xsl:when>
        <xsl:when test="$languageCode='tt'">Tatar</xsl:when>
        <xsl:when test="$languageCode='te'">Telugu</xsl:when>
        <xsl:when test="$languageCode='tg'">Tajik</xsl:when>
        <xsl:when test="$languageCode='tl'">Tagalog</xsl:when>
        <xsl:when test="$languageCode='th'">Thai</xsl:when>
        <xsl:when test="$languageCode='bo'">Tibetan</xsl:when>
        <xsl:when test="$languageCode='ti'">Tigrinya</xsl:when>
        <xsl:when test="$languageCode='to'">Tonga</xsl:when>
        <xsl:when test="$languageCode='tn'">Tswana</xsl:when>
        <xsl:when test="$languageCode='ts'">Tsonga</xsl:when>
        <xsl:when test="$languageCode='tk'">Turkmen</xsl:when>
        <xsl:when test="$languageCode='tr'">Turkish</xsl:when>
        <xsl:when test="$languageCode='tw'">Twi</xsl:when>
        <xsl:when test="$languageCode='ug'">Uighur; Uyghur</xsl:when>
        <xsl:when test="$languageCode='uk'">Ukrainian</xsl:when>
        <xsl:when test="$languageCode='ur'">Urdu</xsl:when>
        <xsl:when test="$languageCode='uz'">Uzbek</xsl:when>
        <xsl:when test="$languageCode='ve'">Venda</xsl:when>
        <xsl:when test="$languageCode='vi'">Vietnamese</xsl:when>
        <xsl:when test="$languageCode='vo'">Volapük</xsl:when>
        <xsl:when test="$languageCode='cy'">Welsh</xsl:when>
        <xsl:when test="$languageCode='wa'">Walloon</xsl:when>
        <xsl:when test="$languageCode='wo'">Wolof</xsl:when>
        <xsl:when test="$languageCode='xh'">Xhosa</xsl:when>
        <xsl:when test="$languageCode='yi'">Yiddish</xsl:when>
        <xsl:when test="$languageCode='yo'">Yoruba</xsl:when>
        <xsl:when test="$languageCode='za'">Zhuang</xsl:when>
        <xsl:when test="$languageCode='zu'">Zulu</xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$languageCode"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="countryName">
      <xsl:choose>
        <xsl:when test="$countryCode='AF'">Afghanistan</xsl:when>
        <xsl:when test="$countryCode='AX'">Åland Islands</xsl:when>
        <xsl:when test="$countryCode='AL'">Albania</xsl:when>
        <xsl:when test="$countryCode='DZ'">Algeria</xsl:when>
        <xsl:when test="$countryCode='AS'">American Samoa</xsl:when>
        <xsl:when test="$countryCode='AD'">Andorra</xsl:when>
        <xsl:when test="$countryCode='AO'">Angola</xsl:when>
        <xsl:when test="$countryCode='AI'">Anguilla</xsl:when>
        <xsl:when test="$countryCode='AQ'">Antarctica</xsl:when>
        <xsl:when test="$countryCode='AG'">Antigua And Barbuda</xsl:when>
        <xsl:when test="$countryCode='AR'">Argentina</xsl:when>
        <xsl:when test="$countryCode='AM'">Armenia</xsl:when>
        <xsl:when test="$countryCode='AW'">Aruba</xsl:when>
        <xsl:when test="$countryCode='AU'">Australia</xsl:when>
        <xsl:when test="$countryCode='AT'">Austria</xsl:when>
        <xsl:when test="$countryCode='AZ'">Azerbaijan</xsl:when>
        <xsl:when test="$countryCode='BS'">Bahamas</xsl:when>
        <xsl:when test="$countryCode='BH'">Bahrain</xsl:when>
        <xsl:when test="$countryCode='BD'">Bangladesh</xsl:when>
        <xsl:when test="$countryCode='BB'">Barbados</xsl:when>
        <xsl:when test="$countryCode='BY'">Belarus</xsl:when>
        <xsl:when test="$countryCode='BE'">Belgium</xsl:when>
        <xsl:when test="$countryCode='BZ'">Belize</xsl:when>
        <xsl:when test="$countryCode='BJ'">Benin</xsl:when>
        <xsl:when test="$countryCode='BM'">Bermuda</xsl:when>
        <xsl:when test="$countryCode='BT'">Bhutan</xsl:when>
        <xsl:when test="$countryCode='BO'">Bolivia</xsl:when>
        <xsl:when test="$countryCode='BA'">Bosnia And Herzegovina</xsl:when>
        <xsl:when test="$countryCode='BW'">Botswana</xsl:when>
        <xsl:when test="$countryCode='BV'">Bouvet Island</xsl:when>
        <xsl:when test="$countryCode='BR'">Brazil</xsl:when>
        <xsl:when test="$countryCode='IO'">British Indian Ocean Territory</xsl:when>
        <xsl:when test="$countryCode='BN'">Brunei Darussalam</xsl:when>
        <xsl:when test="$countryCode='BG'">Bulgaria</xsl:when>
        <xsl:when test="$countryCode='BF'">Burkina Faso</xsl:when>
        <xsl:when test="$countryCode='BI'">Burundi</xsl:when>
        <xsl:when test="$countryCode='KH'">Cambodia</xsl:when>
        <xsl:when test="$countryCode='CM'">Cameroon</xsl:when>
        <xsl:when test="$countryCode='CA'">Canada</xsl:when>
        <xsl:when test="$countryCode='CV'">Cape Verde</xsl:when>
        <xsl:when test="$countryCode='KY'">Cayman Islands</xsl:when>
        <xsl:when test="$countryCode='CF'">Central African Republic</xsl:when>
        <xsl:when test="$countryCode='TD'">Chad</xsl:when>
        <xsl:when test="$countryCode='CL'">Chile</xsl:when>
        <xsl:when test="$countryCode='CN'">China</xsl:when>
        <xsl:when test="$countryCode='CX'">Christmas Island</xsl:when>
        <xsl:when test="$countryCode='CC'">Cocos (Keeling) Islands</xsl:when>
        <xsl:when test="$countryCode='CO'">Colombia</xsl:when>
        <xsl:when test="$countryCode='KM'">Comoros</xsl:when>
        <xsl:when test="$countryCode='CG'">Congo</xsl:when>
        <xsl:when test="$countryCode='CD'">Congo, The Democratic Republic Of The</xsl:when>
        <xsl:when test="$countryCode='CK'">Cook Islands</xsl:when>
        <xsl:when test="$countryCode='CR'">Costa Rica</xsl:when>
        <xsl:when test="$countryCode='CI'">Côte D'Ivoire</xsl:when>
        <xsl:when test="$countryCode='HR'">Croatia</xsl:when>
        <xsl:when test="$countryCode='CU'">Cuba</xsl:when>
        <xsl:when test="$countryCode='CY'">Cyprus</xsl:when>
        <xsl:when test="$countryCode='CZ'">Czech Republic</xsl:when>
        <xsl:when test="$countryCode='DK'">Denmark</xsl:when>
        <xsl:when test="$countryCode='DJ'">Djibouti</xsl:when>
        <xsl:when test="$countryCode='DM'">Dominica</xsl:when>
        <xsl:when test="$countryCode='DO'">Dominican Republic</xsl:when>
        <xsl:when test="$countryCode='EC'">Ecuador</xsl:when>
        <xsl:when test="$countryCode='EG'">Egypt</xsl:when>
        <xsl:when test="$countryCode='SV'">El Salvador</xsl:when>
        <xsl:when test="$countryCode='GQ'">Equatorial Guinea</xsl:when>
        <xsl:when test="$countryCode='ER'">Eritrea</xsl:when>
        <xsl:when test="$countryCode='EE'">Estonia</xsl:when>
        <xsl:when test="$countryCode='ET'">Ethiopia</xsl:when>
        <xsl:when test="$countryCode='FK'">Falkland Islands (Malvinas)</xsl:when>
        <xsl:when test="$countryCode='FO'">Faroe Islands</xsl:when>
        <xsl:when test="$countryCode='FJ'">Fiji</xsl:when>
        <xsl:when test="$countryCode='FI'">Finland</xsl:when>
        <xsl:when test="$countryCode='FR'">France</xsl:when>
        <xsl:when test="$countryCode='GF'">French Guiana</xsl:when>
        <xsl:when test="$countryCode='PF'">French Polynesia</xsl:when>
        <xsl:when test="$countryCode='TF'">French Southern Territories</xsl:when>
        <xsl:when test="$countryCode='GA'">Gabon</xsl:when>
        <xsl:when test="$countryCode='GM'">Gambia</xsl:when>
        <xsl:when test="$countryCode='GE'">Georgia</xsl:when>
        <xsl:when test="$countryCode='DE'">Germany</xsl:when>
        <xsl:when test="$countryCode='GH'">Ghana</xsl:when>
        <xsl:when test="$countryCode='GI'">Gibraltar</xsl:when>
        <xsl:when test="$countryCode='GR'">Greece</xsl:when>
        <xsl:when test="$countryCode='GL'">Greenland</xsl:when>
        <xsl:when test="$countryCode='GD'">Grenada</xsl:when>
        <xsl:when test="$countryCode='GP'">Guadeloupe</xsl:when>
        <xsl:when test="$countryCode='GU'">Guam</xsl:when>
        <xsl:when test="$countryCode='GT'">Guatemala</xsl:when>
        <xsl:when test="$countryCode='GG'">Guernsey</xsl:when>
        <xsl:when test="$countryCode='GN'">Guinea</xsl:when>
        <xsl:when test="$countryCode='GW'">Guinea-Bissau</xsl:when>
        <xsl:when test="$countryCode='GY'">Guyana</xsl:when>
        <xsl:when test="$countryCode='HT'">Haiti</xsl:when>
        <xsl:when test="$countryCode='HM'">Heard Island And Mcdonald Islands</xsl:when>
        <xsl:when test="$countryCode='VA'">Holy See (Vatican City State)</xsl:when>
        <xsl:when test="$countryCode='HN'">Honduras</xsl:when>
        <xsl:when test="$countryCode='HK'">Hong Kong</xsl:when>
        <xsl:when test="$countryCode='HU'">Hungary</xsl:when>
        <xsl:when test="$countryCode='IS'">Iceland</xsl:when>
        <xsl:when test="$countryCode='IN'">India</xsl:when>
        <xsl:when test="$countryCode='ID'">Indonesia</xsl:when>
        <xsl:when test="$countryCode='IR'">Iran, Islamic Republic Of</xsl:when>
        <xsl:when test="$countryCode='IQ'">Iraq</xsl:when>
        <xsl:when test="$countryCode='IE'">Ireland</xsl:when>
        <xsl:when test="$countryCode='IM'">Isle Of Man</xsl:when>
        <xsl:when test="$countryCode='IL'">Israel</xsl:when>
        <xsl:when test="$countryCode='IT'">Italy</xsl:when>
        <xsl:when test="$countryCode='JM'">Jamaica</xsl:when>
        <xsl:when test="$countryCode='JP'">Japan</xsl:when>
        <xsl:when test="$countryCode='JE'">Jersey</xsl:when>
        <xsl:when test="$countryCode='JO'">Jordan</xsl:when>
        <xsl:when test="$countryCode='KZ'">Kazakhstan</xsl:when>
        <xsl:when test="$countryCode='KE'">Kenya</xsl:when>
        <xsl:when test="$countryCode='KI'">Kiribati</xsl:when>
        <xsl:when test="$countryCode='KP'">Korea, Democratic People's Republic Of</xsl:when>
        <xsl:when test="$countryCode='KR'">Korea, Republic Of</xsl:when>
        <xsl:when test="$countryCode='KW'">Kuwait</xsl:when>
        <xsl:when test="$countryCode='KG'">Kyrgyzstan</xsl:when>
        <xsl:when test="$countryCode='LA'">Lao People'S Democratic Republic</xsl:when>
        <xsl:when test="$countryCode='LV'">Latvia</xsl:when>
        <xsl:when test="$countryCode='LB'">Lebanon</xsl:when>
        <xsl:when test="$countryCode='LS'">Lesotho</xsl:when>
        <xsl:when test="$countryCode='LR'">Liberia</xsl:when>
        <xsl:when test="$countryCode='LY'">Libyan Arab Jamahiriya</xsl:when>
        <xsl:when test="$countryCode='LI'">Liechtenstein</xsl:when>
        <xsl:when test="$countryCode='LT'">Lithuania</xsl:when>
        <xsl:when test="$countryCode='LU'">Luxembourg</xsl:when>
        <xsl:when test="$countryCode='MO'">Macao</xsl:when>
        <xsl:when test="$countryCode='MK'">Macedonia, The Former Yugoslav Republic Of</xsl:when>
        <xsl:when test="$countryCode='MG'">Madagascar</xsl:when>
        <xsl:when test="$countryCode='MW'">Malawi</xsl:when>
        <xsl:when test="$countryCode='MY'">Malaysia</xsl:when>
        <xsl:when test="$countryCode='MV'">Maldives</xsl:when>
        <xsl:when test="$countryCode='ML'">Mali</xsl:when>
        <xsl:when test="$countryCode='MT'">Malta</xsl:when>
        <xsl:when test="$countryCode='MH'">Marshall Islands</xsl:when>
        <xsl:when test="$countryCode='MQ'">Martinique</xsl:when>
        <xsl:when test="$countryCode='MR'">Mauritania</xsl:when>
        <xsl:when test="$countryCode='MU'">Mauritius</xsl:when>
        <xsl:when test="$countryCode='YT'">Mayotte</xsl:when>
        <xsl:when test="$countryCode='MX'">Mexico</xsl:when>
        <xsl:when test="$countryCode='FM'">Micronesia, Federated States Of</xsl:when>
        <xsl:when test="$countryCode='MD'">Moldova, Republic Of</xsl:when>
        <xsl:when test="$countryCode='MC'">Monaco</xsl:when>
        <xsl:when test="$countryCode='MN'">Mongolia</xsl:when>
        <xsl:when test="$countryCode='ME'">Montenegro</xsl:when>
        <xsl:when test="$countryCode='MS'">Montserrat</xsl:when>
        <xsl:when test="$countryCode='MA'">Morocco</xsl:when>
        <xsl:when test="$countryCode='MZ'">Mozambique</xsl:when>
        <xsl:when test="$countryCode='MM'">Myanmar</xsl:when>
        <xsl:when test="$countryCode='NA'">Namibia</xsl:when>
        <xsl:when test="$countryCode='NR'">Nauru</xsl:when>
        <xsl:when test="$countryCode='NP'">Nepal</xsl:when>
        <xsl:when test="$countryCode='NL'">Netherlands</xsl:when>
        <xsl:when test="$countryCode='AN'">Netherlands Antilles</xsl:when>
        <xsl:when test="$countryCode='NC'">New Caledonia</xsl:when>
        <xsl:when test="$countryCode='NZ'">New Zealand</xsl:when>
        <xsl:when test="$countryCode='NI'">Nicaragua</xsl:when>
        <xsl:when test="$countryCode='NE'">Niger</xsl:when>
        <xsl:when test="$countryCode='NG'">Nigeria</xsl:when>
        <xsl:when test="$countryCode='NU'">Niue</xsl:when>
        <xsl:when test="$countryCode='NF'">Norfolk Island</xsl:when>
        <xsl:when test="$countryCode='MP'">Northern Mariana Islands</xsl:when>
        <xsl:when test="$countryCode='NO'">Norway</xsl:when>
        <xsl:when test="$countryCode='OM'">Oman</xsl:when>
        <xsl:when test="$countryCode='PK'">Pakistan</xsl:when>
        <xsl:when test="$countryCode='PW'">Palau</xsl:when>
        <xsl:when test="$countryCode='PS'">Palestinian Territory, Occupied</xsl:when>
        <xsl:when test="$countryCode='PA'">Panama</xsl:when>
        <xsl:when test="$countryCode='PG'">Papua New Guinea</xsl:when>
        <xsl:when test="$countryCode='PY'">Paraguay</xsl:when>
        <xsl:when test="$countryCode='PE'">Peru</xsl:when>
        <xsl:when test="$countryCode='PH'">Philippines</xsl:when>
        <xsl:when test="$countryCode='PN'">Pitcairn</xsl:when>
        <xsl:when test="$countryCode='PL'">Poland</xsl:when>
        <xsl:when test="$countryCode='PT'">Portugal</xsl:when>
        <xsl:when test="$countryCode='PR'">Puerto Rico</xsl:when>
        <xsl:when test="$countryCode='QA'">Qatar</xsl:when>
        <xsl:when test="$countryCode='RE'">Réunion</xsl:when>
        <xsl:when test="$countryCode='RO'">Romania</xsl:when>
        <xsl:when test="$countryCode='RU'">Russian Federation</xsl:when>
        <xsl:when test="$countryCode='RW'">Rwanda</xsl:when>
        <xsl:when test="$countryCode='BL'">Saint Barthélemy</xsl:when>
        <xsl:when test="$countryCode='SH'">Saint Helena</xsl:when>
        <xsl:when test="$countryCode='KN'">Saint Kitts And Nevis</xsl:when>
        <xsl:when test="$countryCode='LC'">Saint Lucia</xsl:when>
        <xsl:when test="$countryCode='MF'">Saint Martin</xsl:when>
        <xsl:when test="$countryCode='PM'">Saint Pierre And Miquelon</xsl:when>
        <xsl:when test="$countryCode='VC'">Saint Vincent And The Grenadines</xsl:when>
        <xsl:when test="$countryCode='WS'">Samoa</xsl:when>
        <xsl:when test="$countryCode='SM'">San Marino</xsl:when>
        <xsl:when test="$countryCode='ST'">Sao Tome And Principe</xsl:when>
        <xsl:when test="$countryCode='SA'">Saudi Arabia</xsl:when>
        <xsl:when test="$countryCode='SN'">Senegal</xsl:when>
        <xsl:when test="$countryCode='RS'">Serbia</xsl:when>
        <xsl:when test="$countryCode='SC'">Seychelles</xsl:when>
        <xsl:when test="$countryCode='SL'">Sierra Leone</xsl:when>
        <xsl:when test="$countryCode='SG'">Singapore</xsl:when>
        <xsl:when test="$countryCode='SK'">Slovakia</xsl:when>
        <xsl:when test="$countryCode='SI'">Slovenia</xsl:when>
        <xsl:when test="$countryCode='SB'">Solomon Islands</xsl:when>
        <xsl:when test="$countryCode='SO'">Somalia</xsl:when>
        <xsl:when test="$countryCode='ZA'">South Africa</xsl:when>
        <xsl:when test="$countryCode='GS'">South Georgia And The South Sandwich Islands</xsl:when>
        <xsl:when test="$countryCode='ES'">Spain</xsl:when>
        <xsl:when test="$countryCode='LK'">Sri Lanka</xsl:when>
        <xsl:when test="$countryCode='SD'">Sudan</xsl:when>
        <xsl:when test="$countryCode='SR'">Suriname</xsl:when>
        <xsl:when test="$countryCode='SJ'">Svalbard And Jan Mayen</xsl:when>
        <xsl:when test="$countryCode='SZ'">Swaziland</xsl:when>
        <xsl:when test="$countryCode='SE'">Sweden</xsl:when>
        <xsl:when test="$countryCode='CH'">Switzerland</xsl:when>
        <xsl:when test="$countryCode='SY'">Syrian Arab Republic</xsl:when>
        <xsl:when test="$countryCode='TW'">Taiwan, Province Of China</xsl:when>
        <xsl:when test="$countryCode='TJ'">Tajikistan</xsl:when>
        <xsl:when test="$countryCode='TZ'">Tanzania, United Republic Of</xsl:when>
        <xsl:when test="$countryCode='TH'">Thailand</xsl:when>
        <xsl:when test="$countryCode='TL'">Timor-Leste</xsl:when>
        <xsl:when test="$countryCode='TG'">Togo</xsl:when>
        <xsl:when test="$countryCode='TK'">Tokelau</xsl:when>
        <xsl:when test="$countryCode='TO'">Tonga</xsl:when>
        <xsl:when test="$countryCode='TT'">Trinidad And Tobago</xsl:when>
        <xsl:when test="$countryCode='TN'">Tunisia</xsl:when>
        <xsl:when test="$countryCode='TR'">Turkey</xsl:when>
        <xsl:when test="$countryCode='TM'">Turkmenistan</xsl:when>
        <xsl:when test="$countryCode='TC'">Turks And Caicos Islands</xsl:when>
        <xsl:when test="$countryCode='TV'">Tuvalu</xsl:when>
        <xsl:when test="$countryCode='UG'">Uganda</xsl:when>
        <xsl:when test="$countryCode='UA'">Ukraine</xsl:when>
        <xsl:when test="$countryCode='AE'">United Arab Emirates</xsl:when>
        <xsl:when test="$countryCode='GB'">United Kingdom</xsl:when>
        <xsl:when test="$countryCode='US'">United States</xsl:when>
        <xsl:when test="$countryCode='UM'">United States Minor Outlying Islands</xsl:when>
        <xsl:when test="$countryCode='UY'">Uruguay</xsl:when>
        <xsl:when test="$countryCode='UZ'">Uzbekistan</xsl:when>
        <xsl:when test="$countryCode='VU'">Vanuatu</xsl:when>
        <xsl:when test="$countryCode='VE'">Venezuela, Bolivarian Republic Of</xsl:when>
        <xsl:when test="$countryCode='VN'">Viet Nam</xsl:when>
        <xsl:when test="$countryCode='VG'">Virgin Islands, British</xsl:when>
        <xsl:when test="$countryCode='VI'">Virgin Islands, U.S.</xsl:when>
        <xsl:when test="$countryCode='WF'">Wallis And Futuna</xsl:when>
        <xsl:when test="$countryCode='EH'">Western Sahara</xsl:when>
        <xsl:when test="$countryCode='YE'">Yemen</xsl:when>
        <xsl:when test="$countryCode='ZM'">Zambia</xsl:when>
        <xsl:when test="$countryCode='ZW'">Zimbabwe</xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$countryCode"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:value-of select="$languageName"/>
    <xsl:if test="$countryName!=''">
      <xsl:text> (</xsl:text>
      <xsl:value-of select="$countryName"/>
      <xsl:text>)</xsl:text>
    </xsl:if>
  </xsl:template>

  <!-- END Component text rendering -->
</xsl:stylesheet>