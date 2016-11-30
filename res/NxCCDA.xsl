<?xml version="1.0" encoding="UTF-8"?>
<!--
  Title: CDA XSL StyleSheet
  Original Filename: cda.xsl 
  Version: 3.0
  Revision History: 08/12/08 Jingdong Li updated
  Revision History: 12/11/09 KH updated 
  Revision History:  03/30/10 Jingdong Li updated.
  Revision History:  08/25/10 Jingdong Li updated
  Revision History:  09/17/10 Jingdong Li updated
  Revision History:  01/05/11 Jingdong Li updated
  Revision History:  10/09/12 Lauren Wood minor updates
  Specification: ANSI/HL7 CDAR2  
  The current version and documentation are available at http://www.lantanagroup.com/resources/tools/. 
  We welcome feedback and contributions to tools@lantanagroup.com
  The stylesheet is the cumulative work of several developers; the most significant prior milestones were the foundation work from HL7 
  Germany and Finland (Tyylitiedosto) and HL7 US (Calvin Beebe), and the presentation approach from Tony Schaller, medshare GmbH provided at IHIC 2009. 
-->
<!-- LICENSE INFORMATION
  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
  You may obtain a copy of the License at  http://www.apache.org/licenses/LICENSE-2.0 
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:n1="urn:hl7-org:v3" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:nx="http://www.nextech.com/xslExtensions" xmlns:msxsl="urn:schemas-microsoft-com:xslt">
   <xsl:output method="html" indent="yes" version="4.01" encoding="ISO-8859-1" doctype-system="http://www.w3.org/TR/html4/strict.dtd" doctype-public="-//W3C//DTD HTML 4.01//EN"/>
   <!-- global variable title -->
   <xsl:variable name="title">
      <xsl:choose>
         <xsl:when test="string-length(/n1:ClinicalDocument/n1:title)  &gt;= 1">
            <xsl:value-of select="/n1:ClinicalDocument/n1:title"/>
         </xsl:when>
         <xsl:when test="/n1:ClinicalDocument/n1:code/@displayName">
            <xsl:value-of select="/n1:ClinicalDocument/n1:code/@displayName"/>
         </xsl:when>
         <xsl:otherwise>
            <xsl:text>Clinical Document</xsl:text>
         </xsl:otherwise>
      </xsl:choose>
   </xsl:variable>
   <!-- Main -->
   <xsl:template match="/">
      <xsl:apply-templates select="n1:ClinicalDocument"/>
   </xsl:template>
   <!-- produce browser rendered, human readable clinical document -->
   <xsl:template match="n1:ClinicalDocument">
      <html>
         <head>
      <!-- (b.spivey - October 16th, 2013) - PLID 59066 - java script to hide/show sections on click. -->
            <script language="javascript" defer="true">
                 <xsl:comment>
                  <![CDATA[ 
                    function toggleDisplay(title)
                     {
                     
                     if (document.getElementById("check_" + title).checked == false)
                       document.getElementById(title).style.display="none";
                                              
                     if (document.getElementById("check_" + title).checked == true)
                       document.getElementById(title).style.display="block"; 
                       
                     }
                  ]]> 
                </xsl:comment>
            </script>
            <xsl:comment> Do NOT edit this HTML directly: it was generated via an XSLT transformation from a CDA Release 2 XML document. </xsl:comment>
            <title>
               <xsl:value-of select="$title"/>
            </title>
            <xsl:call-template name="addCSS"/>
         </head>
         <body>
            <h1 class="h1center">
               <xsl:value-of select="$title"/>
            </h1>
            <!-- START display top portion of clinical document -->
            <xsl:call-template name="recordTarget"/>
            <xsl:call-template name="documentGeneral"/>
            <xsl:call-template name="documentationOf"/>
            <xsl:call-template name="author"/>
            <xsl:call-template name="componentof"/>
            <xsl:call-template name="participant"/>
            <xsl:call-template name="dataEnterer"/>
            <xsl:call-template name="authenticator"/>
            <xsl:call-template name="informant"/>
            <xsl:call-template name="informationRecipient"/>
            <xsl:call-template name="legalAuthenticator"/>
            <xsl:call-template name="custodian"/>
            <!-- END display top portion of clinical document -->
            <!-- produce table of contents -->
            <xsl:if test="not(//n1:nonXMLBody)">
               <xsl:if test="count(/n1:ClinicalDocument/n1:component/n1:structuredBody/n1:component[n1:section]) &gt; 1">
                  <xsl:call-template name="make-tableofcontents"/>
               </xsl:if>
            </xsl:if>
            <hr align="left" color="teal" size="2" width="80%"/>
            <!-- produce human readable document content -->
            <xsl:apply-templates select="n1:component/n1:structuredBody|n1:component/n1:nonXMLBody"/>
            <br/>
            <br/>
         </body>
      </html>
   </xsl:template>
   <!-- generate table of contents -->
   <xsl:template name="make-tableofcontents">
      <h2>
         <a name="toc">Table of Contents</a>
      </h2>
      <!-- (b.spivey - October 16th, 2013) - PLID 59066 - generate check boxes, default them to true.-->
      <!-- (j.gruber 2014-04-28) - PLID 61848 - renaming the instructions-->
      <p> You can uncheck boxes for easier viewing. It does not save and will reset upon closing. </p>
      <ul id="toc">
         <xsl:for-each select="n1:component/n1:structuredBody/n1:component/n1:section/n1:title">
            <li>
                <input type="checkbox" id="check_{generate-id(.)}" onclick="toggleDisplay('{generate-id(.)}')" >
                    <xsl:attribute name="checked">checked</xsl:attribute>
                </input>
                <a href="#{generate-id(.)}">    
                    <xsl:value-of select="."/>
                </a>
            </li>
         </xsl:for-each>
      </ul>
   </xsl:template>
   <!-- header elements -->
   <xsl:template name="documentGeneral">
      <table class="header_table">
         <tbody>
            <tr>
               <td width="20%" bgcolor="#3399ff">
                  <span class="td_label">
                     <xsl:text>Document Id</xsl:text>
                  </span>
               </td>
               <td width="80%">
                  <xsl:call-template name="show-id">
                     <xsl:with-param name="id" select="n1:id"/>
                  </xsl:call-template>
               </td>
            </tr>
            <tr>
               <td width="20%" bgcolor="#3399ff">
                  <span class="td_label">
                     <xsl:text>Document Created</xsl:text>
                  </span>
               </td>
               <td width="80%">
                  <xsl:call-template name="show-time">
                     <xsl:with-param name="datetime" select="n1:effectiveTime"/>
                  </xsl:call-template>
               </td>
            </tr>
         </tbody>
      </table>
   </xsl:template>
   <!-- confidentiality -->
   <xsl:template name="confidentiality">
      <table class="header_table">
         <tbody>
            <td width="20%" bgcolor="#3399ff">
               <xsl:text>Confidentiality</xsl:text>
            </td>
            <td width="80%">
               <xsl:choose>
                  <xsl:when test="n1:confidentialityCode/@code  = &apos;N&apos;">
                     <xsl:text>Normal</xsl:text>
                  </xsl:when>
                  <xsl:when test="n1:confidentialityCode/@code  = &apos;R&apos;">
                     <xsl:text>Restricted</xsl:text>
                  </xsl:when>
                  <xsl:when test="n1:confidentialityCode/@code  = &apos;V&apos;">
                     <xsl:text>Very restricted</xsl:text>
                  </xsl:when>
               </xsl:choose>
               <xsl:if test="n1:confidentialityCode/n1:originalText">
                  <xsl:text> </xsl:text>
                  <xsl:value-of select="n1:confidentialityCode/n1:originalText"/>
               </xsl:if>
            </td>
         </tbody>
      </table>
   </xsl:template>
   <!-- author -->
   <xsl:template name="author">
      <xsl:if test="n1:author">
         <table class="header_table">
            <tbody>
               <xsl:for-each select="n1:author/n1:assignedAuthor">
                  <tr>
                     <td width="20%" bgcolor="#3399ff">
                        <span class="td_label">
                           <xsl:text>Author</xsl:text>
                        </span>
                     </td>
                     <td width="80%">
                        <xsl:choose>
                           <xsl:when test="n1:assignedPerson/n1:name">
                              <xsl:call-template name="show-name">
                                 <xsl:with-param name="name" select="n1:assignedPerson/n1:name"/>
                              </xsl:call-template>
                              <xsl:if test="n1:representedOrganization">
                                 <xsl:text>, </xsl:text>
                                 <xsl:call-template name="show-name">
                                    <xsl:with-param name="name" select="n1:representedOrganization/n1:name"/>
                                 </xsl:call-template>
                              </xsl:if>
                           </xsl:when>
                           <xsl:when test="n1:assignedAuthoringDevice/n1:softwareName">
                              <xsl:value-of select="n1:assignedAuthoringDevice/n1:softwareName"/>
                           </xsl:when>
                           <xsl:when test="n1:representedOrganization">
                              <xsl:call-template name="show-name">
                                 <xsl:with-param name="name" select="n1:representedOrganization/n1:name"/>
                              </xsl:call-template>
                           </xsl:when>
                           <xsl:otherwise>
                              <xsl:for-each select="n1:id">
                                 <xsl:call-template name="show-id"/>
                                 <br/>
                              </xsl:for-each>
                           </xsl:otherwise>
                        </xsl:choose>
                     </td>
                  </tr>
                  <xsl:if test="n1:addr | n1:telecom">
                     <tr>
                        <td bgcolor="#3399ff">
                           <span class="td_label">
                              <xsl:text>Contact info</xsl:text>
                           </span>
                        </td>
                        <td>
                           <xsl:call-template name="show-contactInfo">
                              <xsl:with-param name="contact" select="."/>
                           </xsl:call-template>
                        </td>
                     </tr>
                  </xsl:if>
               </xsl:for-each>
            </tbody>
         </table>
      </xsl:if>
   </xsl:template>
   <!--  authenticator -->
   <xsl:template name="authenticator">
      <xsl:if test="n1:authenticator">
         <table class="header_table">
            <tbody>
                  <xsl:for-each select="n1:authenticator">
                     <tr>
                        <td width="20%" bgcolor="#3399ff">
                           <span class="td_label">
                              <xsl:text>Signed</xsl:text>
                           </span>
                        </td>
                        <td width="80%">
                           <xsl:call-template name="show-name">
                              <xsl:with-param name="name" select="n1:assignedEntity/n1:assignedPerson/n1:name"/>
                           </xsl:call-template>
                           <xsl:text> at </xsl:text>
                           <xsl:call-template name="show-time">
                              <xsl:with-param name="date" select="n1:time"/>
                           </xsl:call-template>
                        </td>
                     </tr>
                     <xsl:if test="n1:assignedEntity/n1:addr | n1:assignedEntity/n1:telecom">
                        <tr>
                           <td bgcolor="#3399ff">
                              <span class="td_label">
                                 <xsl:text>Contact info</xsl:text>
                              </span>
                           </td>
                           <td>
                              <xsl:call-template name="show-contactInfo">
                                 <xsl:with-param name="contact" select="n1:assignedEntity"/>
                              </xsl:call-template>
                           </td>
                        </tr>
                     </xsl:if>
                  </xsl:for-each>
            </tbody>
         </table>
      </xsl:if>
   </xsl:template>
   <!-- legalAuthenticator -->
   <xsl:template name="legalAuthenticator">
      <xsl:if test="n1:legalAuthenticator">
         <table class="header_table">
            <tbody>
               <tr>
                  <td width="20%" bgcolor="#3399ff">
                     <span class="td_label">
                        <xsl:text>Legal authenticator</xsl:text>
                     </span>
                  </td>
                  <td width="80%">
                     <xsl:call-template name="show-assignedEntity">
                        <xsl:with-param name="asgnEntity" select="n1:legalAuthenticator/n1:assignedEntity"/>
                     </xsl:call-template>
                     <xsl:text> </xsl:text>
                     <xsl:call-template name="show-sig">
                        <xsl:with-param name="sig" select="n1:legalAuthenticator/n1:signatureCode"/>
                     </xsl:call-template>
                     <xsl:if test="n1:legalAuthenticator/n1:time/@value">
                        <xsl:text> at </xsl:text>
                        <xsl:call-template name="show-time">
                           <xsl:with-param name="datetime" select="n1:legalAuthenticator/n1:time"/>
                        </xsl:call-template>
                     </xsl:if>
                  </td>
               </tr>
               <xsl:if test="n1:legalAuthenticator/n1:assignedEntity/n1:addr | n1:legalAuthenticator/n1:assignedEntity/n1:telecom">
                  <tr>
                     <td bgcolor="#3399ff">
                        <span class="td_label">
                           <xsl:text>Contact info</xsl:text>
                        </span>
                     </td>
                     <td>
                        <xsl:call-template name="show-contactInfo">
                           <xsl:with-param name="contact" select="n1:legalAuthenticator/n1:assignedEntity"/>
                        </xsl:call-template>
                     </td>
                  </tr>
               </xsl:if>
            </tbody>
         </table>
      </xsl:if>
   </xsl:template>
   <!-- dataEnterer -->
   <xsl:template name="dataEnterer">
      <xsl:if test="n1:dataEnterer">
         <table class="header_table">
            <tbody>
               <tr>
                  <td width="20%" bgcolor="#3399ff">
                     <span class="td_label">
                        <xsl:text>Entered by</xsl:text>
                     </span>
                  </td>
                  <td width="80%">
                     <xsl:call-template name="show-assignedEntity">
                        <xsl:with-param name="asgnEntity" select="n1:dataEnterer/n1:assignedEntity"/>
                     </xsl:call-template>
                  </td>
               </tr>
               <xsl:if test="n1:dataEnterer/n1:assignedEntity/n1:addr | n1:dataEnterer/n1:assignedEntity/n1:telecom">
                  <tr>
                     <td bgcolor="#3399ff">
                        <span class="td_label">
                           <xsl:text>Contact info</xsl:text>
                        </span>
                     </td>
                     <td>
                        <xsl:call-template name="show-contactInfo">
                           <xsl:with-param name="contact" select="n1:dataEnterer/n1:assignedEntity"/>
                        </xsl:call-template>
                     </td>
                  </tr>
               </xsl:if>
            </tbody>
         </table>
      </xsl:if>
   </xsl:template>
   <!-- componentOf -->
   <xsl:template name="componentof">
      <xsl:if test="n1:componentOf">
         <table class="header_table">
            <tbody>
               <xsl:for-each select="n1:componentOf/n1:encompassingEncounter">
                  <xsl:if test="n1:id">
                     <tr>
                        <xsl:choose>
                           <xsl:when test="n1:code">
                              <td width="20%" bgcolor="#3399ff">
                                 <span class="td_label">
                                    <xsl:text>Encounter Id</xsl:text>
                                 </span>
                              </td>
                              <td width="30%">
                                 <xsl:call-template name="show-id">
                                    <xsl:with-param name="id" select="n1:id"/>
                                 </xsl:call-template>
                              </td>
                              <td width="15%" bgcolor="#3399ff">
                                 <span class="td_label">
                                    <xsl:text>Encounter Type</xsl:text>
                                 </span>
                              </td>
                              <td>
                                 <xsl:call-template name="show-code">
                                    <xsl:with-param name="code" select="n1:code"/>
                                 </xsl:call-template>
                              </td>
                           </xsl:when>
                           <xsl:otherwise>
                              <td width="20%" bgcolor="#3399ff">
                                 <span class="td_label">
                                    <xsl:text>Encounter Id</xsl:text>
                                 </span>
                              </td>
                              <td>
                                 <xsl:call-template name="show-id">
                                    <xsl:with-param name="id" select="n1:id"/>
                                 </xsl:call-template>
                              </td>
                           </xsl:otherwise>
                        </xsl:choose>
                     </tr>
                  </xsl:if>
                  <tr>
                     <td width="20%" bgcolor="#3399ff">
                        <span class="td_label">
                           <xsl:text>Encounter Date</xsl:text>
                        </span>
                     </td>
                     <td colspan="3">
                        <xsl:if test="n1:effectiveTime">
                           <xsl:choose>
                              <xsl:when test="n1:effectiveTime/@value">
                                 <xsl:call-template name="show-time">
                                    <xsl:with-param name="datetime" select="n1:effectiveTime"/>
                                 </xsl:call-template>
                              </xsl:when>
                              <xsl:when test="n1:effectiveTime/n1:low">
                                 <xsl:text>&#160;From&#160;</xsl:text>
                                 <xsl:call-template name="show-time">
                                    <xsl:with-param name="datetime" select="n1:effectiveTime/n1:low"/>
                                 </xsl:call-template>
                                 <xsl:if test="n1:effectiveTime/n1:high">
                                    <xsl:text> to </xsl:text>
                                    <xsl:call-template name="show-time">
                                       <xsl:with-param name="datetime" select="n1:effectiveTime/n1:high"/>
                                    </xsl:call-template>
                                 </xsl:if>
                              </xsl:when>
                           </xsl:choose>
                        </xsl:if>
                     </td>
                  </tr>
                  <xsl:if test="n1:location/n1:healthCareFacility">
                     <tr>
                        <td width="20%" bgcolor="#3399ff">
                           <span class="td_label">
                              <xsl:text>Encounter Location</xsl:text>
                           </span>
                        </td>
                        <td colspan="3">
                           <xsl:choose>
                              <xsl:when test="n1:location/n1:healthCareFacility/n1:location/n1:name">
                                 <xsl:call-template name="show-name">
                                    <xsl:with-param name="name" select="n1:location/n1:healthCareFacility/n1:location/n1:name"/>
                                 </xsl:call-template>
                                 <xsl:for-each select="n1:location/n1:healthCareFacility/n1:serviceProviderOrganization/n1:name">
                                    <xsl:if test=". != ../../n1:location/n1:name">
                                        <xsl:text> (Organization: </xsl:text>
                                        <xsl:call-template name="show-name">
                                           <xsl:with-param name="name" select="."/>
                                        </xsl:call-template>
                                        <xsl:text>) </xsl:text>
                                    </xsl:if>
                                 </xsl:for-each>
                              </xsl:when>
                              <xsl:when test="n1:location/n1:healthCareFacility/n1:code">
                                 <xsl:call-template name="show-code">
                                    <xsl:with-param name="code" select="n1:location/n1:healthCareFacility/n1:code"/>
                                 </xsl:call-template>
                              </xsl:when>
                              <xsl:otherwise>
                                 <xsl:if test="n1:location/n1:healthCareFacility/n1:id">
                                    <xsl:text>id: </xsl:text>
                                    <xsl:for-each select="n1:location/n1:healthCareFacility/n1:id">
                                       <xsl:call-template name="show-id">
                                          <xsl:with-param name="id" select="."/>
                                       </xsl:call-template>
                                    </xsl:for-each>
                                 </xsl:if>
                              </xsl:otherwise>
                           </xsl:choose>
                        </td>
                     </tr>
                  </xsl:if>
                  <xsl:if test="n1:responsibleParty">
                     <tr>
                        <td width="20%" bgcolor="#3399ff">
                           <span class="td_label">
                              <xsl:text>Responsible party</xsl:text>
                           </span>
                        </td>
                        <td colspan="3">
                           <xsl:call-template name="show-assignedEntity">
                              <xsl:with-param name="asgnEntity" select="n1:responsibleParty/n1:assignedEntity"/>
                           </xsl:call-template>
                        </td>
                     </tr>
                  </xsl:if>
                  <xsl:if test="n1:responsibleParty/n1:assignedEntity/n1:addr | n1:responsibleParty/n1:assignedEntity/n1:telecom">
                     <tr>
                        <td bgcolor="#3399ff">
                           <span class="td_label">
                              <xsl:text>Contact info</xsl:text>
                           </span>
                        </td>
                        <td colspan="3">
                           <xsl:call-template name="show-contactInfo">
                              <xsl:with-param name="contact" select="n1:responsibleParty/n1:assignedEntity"/>
                           </xsl:call-template>
                        </td>
                     </tr>
                  </xsl:if>
               </xsl:for-each>
            </tbody>
         </table>
      </xsl:if>
   </xsl:template>
   <!-- custodian -->
   <xsl:template name="custodian">
      <xsl:if test="n1:custodian">
         <table class="header_table">
            <tbody>
               <tr>
                  <td width="20%" bgcolor="#3399ff">
                     <span class="td_label">
                        <xsl:text>Document maintained by</xsl:text>
                     </span>
                  </td>
                  <td width="80%">
                     <xsl:choose>
                        <xsl:when test="n1:custodian/n1:assignedCustodian/n1:representedCustodianOrganization/n1:name">
                           <xsl:call-template name="show-name">
                              <xsl:with-param name="name" select="n1:custodian/n1:assignedCustodian/n1:representedCustodianOrganization/n1:name"/>
                           </xsl:call-template>
                        </xsl:when>
                        <xsl:otherwise>
                           <xsl:for-each select="n1:custodian/n1:assignedCustodian/n1:representedCustodianOrganization/n1:id">
                              <xsl:call-template name="show-id"/>
                              <xsl:if test="position()!=last()">
                                 <br/>
                              </xsl:if>
                           </xsl:for-each>
                        </xsl:otherwise>
                     </xsl:choose>
                  </td>
               </tr>
               <xsl:if test="n1:custodian/n1:assignedCustodian/n1:representedCustodianOrganization/n1:addr |             n1:custodian/n1:assignedCustodian/n1:representedCustodianOrganization/n1:telecom">
                  <tr>
                     <td bgcolor="#3399ff">
                        <span class="td_label">
                           <xsl:text>Contact info</xsl:text>
                        </span>
                     </td>
                     <td width="80%">
                        <xsl:call-template name="show-contactInfo">
                           <xsl:with-param name="contact" select="n1:custodian/n1:assignedCustodian/n1:representedCustodianOrganization"/>
                        </xsl:call-template>
                     </td>
                  </tr>
               </xsl:if>
            </tbody>
         </table>
      </xsl:if>
   </xsl:template>
   <!-- documentationOf -->
   <xsl:template name="documentationOf">
      <xsl:if test="n1:documentationOf">
         <table class="header_table">
            <tbody>
               <xsl:for-each select="n1:documentationOf">
                  <xsl:if test="n1:serviceEvent/@classCode and n1:serviceEvent/n1:code">
                     <xsl:variable name="displayName">
                        <xsl:call-template name="show-actClassCode">
                           <xsl:with-param name="clsCode" select="n1:serviceEvent/@classCode"/>
                        </xsl:call-template>
                     </xsl:variable>
                     <xsl:if test="$displayName">
                        <tr>
                           <td width="20%" bgcolor="#3399ff">
                              <span class="td_label">
                                 <xsl:call-template name="firstCharCaseUp">
                                    <xsl:with-param name="data" select="$displayName"/>
                                 </xsl:call-template>
                              </span>
                           </td>
                           <td width="80%" colspan="3">
                              <xsl:call-template name="show-code">
                                 <xsl:with-param name="code" select="n1:serviceEvent/n1:code"/>
                              </xsl:call-template>
                              <xsl:if test="n1:serviceEvent/n1:effectiveTime">
                                 <xsl:choose>
                                    <xsl:when test="n1:serviceEvent/n1:effectiveTime/@value">
                                       <xsl:text>&#160;at&#160;</xsl:text>
                                       <xsl:call-template name="show-time">
                                          <xsl:with-param name="datetime" select="n1:serviceEvent/n1:effectiveTime"/>
                                       </xsl:call-template>
                                    </xsl:when>
                                    <xsl:when test="n1:serviceEvent/n1:effectiveTime/n1:low">
                                       <xsl:text>&#160;from&#160;</xsl:text>
                                       <xsl:call-template name="show-time">
                                          <xsl:with-param name="datetime" select="n1:serviceEvent/n1:effectiveTime/n1:low"/>
                                       </xsl:call-template>
                                       <xsl:if test="n1:serviceEvent/n1:effectiveTime/n1:high">
                                          <xsl:text> to </xsl:text>
                                          <xsl:call-template name="show-time">
                                             <xsl:with-param name="datetime" select="n1:serviceEvent/n1:effectiveTime/n1:high"/>
                                          </xsl:call-template>
                                       </xsl:if>
                                    </xsl:when>
                                 </xsl:choose>
                              </xsl:if>
                           </td>
                        </tr>
                     </xsl:if>
                  </xsl:if>
                  <xsl:for-each select="n1:serviceEvent/n1:performer">
                     <xsl:variable name="displayName">
                        <xsl:call-template name="show-participationType">
                           <xsl:with-param name="ptype" select="@typeCode"/>
                        </xsl:call-template>
                        <xsl:text> </xsl:text>
                        <xsl:if test="n1:functionCode/@code">
                           <xsl:call-template name="show-participationFunction">
                              <xsl:with-param name="pFunction" select="n1:functionCode/@code"/>
                           </xsl:call-template>
                        </xsl:if>
                     </xsl:variable>
                     <tr>
                        <td width="20%" bgcolor="#3399ff">
                           <span class="td_label">
                              <xsl:call-template name="firstCharCaseUp">
                                 <xsl:with-param name="data" select="$displayName"/>
                              </xsl:call-template>
                           </span>
                        </td>
                        <td width="80%" colspan="3">
                           <xsl:call-template name="show-assignedEntity">
                              <xsl:with-param name="asgnEntity" select="n1:assignedEntity"/>
                           </xsl:call-template>
                        </td>
                     </tr>
                  </xsl:for-each>
               </xsl:for-each>
            </tbody>
         </table>
      </xsl:if>
   </xsl:template>
   <!-- inFulfillmentOf -->
   <xsl:template name="inFulfillmentOf">
      <xsl:if test="n1:infulfillmentOf">
         <table class="header_table">
            <tbody>
               <xsl:for-each select="n1:inFulfillmentOf">
                  <tr>
                     <td width="20%" bgcolor="#3399ff">
                        <span class="td_label">
                           <xsl:text>In fulfillment of</xsl:text>
                        </span>
                     </td>
                     <td width="80%">
                        <xsl:for-each select="n1:order">
                           <xsl:for-each select="n1:id">
                              <xsl:call-template name="show-id"/>
                           </xsl:for-each>
                           <xsl:for-each select="n1:code">
                              <xsl:text>&#160;</xsl:text>
                              <xsl:call-template name="show-code">
                                 <xsl:with-param name="code" select="."/>
                              </xsl:call-template>
                           </xsl:for-each>
                           <xsl:for-each select="n1:priorityCode">
                              <xsl:text>&#160;</xsl:text>
                              <xsl:call-template name="show-code">
                                 <xsl:with-param name="code" select="."/>
                              </xsl:call-template>
                           </xsl:for-each>
                        </xsl:for-each>
                     </td>
                  </tr>
               </xsl:for-each>
            </tbody>
         </table>
      </xsl:if>
   </xsl:template>
   <!-- informant -->
   <xsl:template name="informant">
      <xsl:if test="n1:informant">
         <table class="header_table">
            <tbody>
               <xsl:for-each select="n1:informant">
                  <tr>
                     <td width="20%" bgcolor="#3399ff">
                        <span class="td_label">
                           <xsl:text>Informant</xsl:text>
                        </span>
                     </td>
                     <td width="80%">
                        <xsl:if test="n1:assignedEntity">
                           <xsl:call-template name="show-assignedEntity">
                              <xsl:with-param name="asgnEntity" select="n1:assignedEntity"/>
                           </xsl:call-template>
                        </xsl:if>
                        <xsl:if test="n1:relatedEntity">
                           <xsl:call-template name="show-relatedEntity">
                              <xsl:with-param name="relatedEntity" select="n1:relatedEntity"/>
                           </xsl:call-template>
                        </xsl:if>
                     </td>
                  </tr>
                  <xsl:choose>
                     <xsl:when test="n1:assignedEntity/n1:addr | n1:assignedEntity/n1:telecom">
                        <tr>
                           <td bgcolor="#3399ff">
                              <span class="td_label">
                                 <xsl:text>Contact info</xsl:text>
                              </span>
                           </td>
                           <td>
                              <xsl:if test="n1:assignedEntity">
                                 <xsl:call-template name="show-contactInfo">
                                    <xsl:with-param name="contact" select="n1:assignedEntity"/>
                                 </xsl:call-template>
                              </xsl:if>
                           </td>
                        </tr>
                     </xsl:when>
                     <xsl:when test="n1:relatedEntity/n1:addr | n1:relatedEntity/n1:telecom">
                        <tr>
                           <td bgcolor="#3399ff">
                              <span class="td_label">
                                 <xsl:text>Contact info</xsl:text>
                              </span>
                           </td>
                           <td>
                              <xsl:if test="n1:relatedEntity">
                                 <xsl:call-template name="show-contactInfo">
                                    <xsl:with-param name="contact" select="n1:relatedEntity"/>
                                 </xsl:call-template>
                              </xsl:if>
                           </td>
                        </tr>
                     </xsl:when>
                  </xsl:choose>
               </xsl:for-each>
            </tbody>
         </table>
      </xsl:if>
   </xsl:template>
   <!-- informantionRecipient -->
   <xsl:template name="informationRecipient">
      <xsl:if test="n1:informationRecipient">
         <table class="header_table">
            <tbody>
               <xsl:for-each select="n1:informationRecipient">
                  <tr>
                     <td width="20%" bgcolor="#3399ff">
                        <span class="td_label">
                           <xsl:text>Information recipient</xsl:text>
                        </span>
                     </td>
                     <td width="80%">
                        <xsl:choose>
                           <xsl:when test="n1:intendedRecipient/n1:informationRecipient/n1:name">
                              <xsl:for-each select="n1:intendedRecipient/n1:informationRecipient">
                                 <xsl:call-template name="show-name">
                                    <xsl:with-param name="name" select="n1:name"/>
                                 </xsl:call-template>
                                 <xsl:if test="position() != last()">
                                    <br/>
                                 </xsl:if>
                              </xsl:for-each>
                           </xsl:when>
                           <xsl:otherwise>
                              <xsl:for-each select="n1:intendedRecipient">
                                 <xsl:for-each select="n1:id">
                                    <xsl:call-template name="show-id"/>
                                 </xsl:for-each>
                                 <xsl:if test="position() != last()">
                                    <br/>
                                 </xsl:if>
                                 <br/>
                              </xsl:for-each>
                           </xsl:otherwise>
                        </xsl:choose>
                     </td>
                  </tr>
                  <xsl:if test="n1:intendedRecipient/n1:addr | n1:intendedRecipient/n1:telecom">
                     <tr>
                        <td bgcolor="#3399ff">
                           <span class="td_label">
                              <xsl:text>Contact info</xsl:text>
                           </span>
                        </td>
                        <td>
                           <xsl:call-template name="show-contactInfo">
                              <xsl:with-param name="contact" select="n1:intendedRecipient"/>
                           </xsl:call-template>
                        </td>
                     </tr>
                  </xsl:if>
               </xsl:for-each>
            </tbody>
         </table>
      </xsl:if>
   </xsl:template>
   <!-- participant -->
   <xsl:template name="participant">
      <xsl:if test="n1:participant">
         <table class="header_table">
            <tbody>
               <xsl:for-each select="n1:participant">
                  <tr>
                     <td width="20%" bgcolor="#3399ff">
                        <xsl:variable name="participtRole">
                           <xsl:call-template name="translateRoleAssoCode">
                              <xsl:with-param name="classCode" select="n1:associatedEntity/@classCode"/>
                              <xsl:with-param name="code" select="n1:associatedEntity/n1:code"/>
                           </xsl:call-template>
                        </xsl:variable>
                        <xsl:choose>
                           <xsl:when test="$participtRole">
                              <span class="td_label">
                                 <xsl:call-template name="firstCharCaseUp">
                                    <xsl:with-param name="data" select="$participtRole"/>
                                 </xsl:call-template>
                              </span>
                           </xsl:when>
                           <xsl:otherwise>
                              <span class="td_label">
                                 <xsl:text>Participant</xsl:text>
                              </span>
                           </xsl:otherwise>
                        </xsl:choose>
                     </td>
                     <td width="80%">
                        <xsl:if test="n1:functionCode">
                           <xsl:call-template name="show-code">
                              <xsl:with-param name="code" select="n1:functionCode"/>
                           </xsl:call-template>
                           <xsl:text> </xsl:text>
                        </xsl:if>
                        <xsl:call-template name="show-associatedEntity">
                           <xsl:with-param name="assoEntity" select="n1:associatedEntity"/>
                        </xsl:call-template>
                        <xsl:if test="n1:time">
                           <xsl:if test="n1:time/n1:low">
                              <xsl:text> from </xsl:text>
                              <xsl:call-template name="show-time">
                                 <xsl:with-param name="datetime" select="n1:time/n1:low"/>
                              </xsl:call-template>
                           </xsl:if>
                           <xsl:if test="n1:time/n1:high">
                              <xsl:text> to </xsl:text>
                              <xsl:call-template name="show-time">
                                 <xsl:with-param name="datetime" select="n1:time/n1:high"/>
                              </xsl:call-template>
                           </xsl:if>
                        </xsl:if>
                        <xsl:if test="position() != last()">
                           <br/>
                        </xsl:if>
                     </td>
                  </tr>
                  <xsl:if test="n1:associatedEntity/n1:addr | n1:associatedEntity/n1:telecom">
                     <tr>
                        <td bgcolor="#3399ff">
                           <span class="td_label">
                              <xsl:text>Contact info</xsl:text>
                           </span>
                        </td>
                        <td>
                           <xsl:call-template name="show-contactInfo">
                              <xsl:with-param name="contact" select="n1:associatedEntity"/>
                           </xsl:call-template>
                        </td>
                     </tr>
                  </xsl:if>
               </xsl:for-each>
            </tbody>
         </table>
      </xsl:if>
   </xsl:template>
   <!-- recordTarget -->
   <xsl:template name="recordTarget">
      <table class="header_table">
         <tbody>
            <xsl:for-each select="/n1:ClinicalDocument/n1:recordTarget/n1:patientRole">
               <xsl:if test="not(n1:id/@nullFlavor)">
                  <tr>
                     <td width="20%" bgcolor="#3399ff">
                        <span class="td_label">
                           <xsl:text>Patient</xsl:text>
                        </span>
                     </td>
                     <td colspan="3">
                        <xsl:call-template name="show-name">
                           <xsl:with-param name="name" select="n1:patient/n1:name"/>
                        </xsl:call-template>
                     </td>
                  </tr>
                  <tr>
                     <td width="20%" bgcolor="#3399ff">
                        <span class="td_label">
                           <xsl:text>Date of birth</xsl:text>
                        </span>
                     </td>
                     <td width="30%">
                        <xsl:call-template name="show-time">
                           <xsl:with-param name="datetime" select="n1:patient/n1:birthTime"/>
                        </xsl:call-template>
                     </td>
                     <td width="15%" bgcolor="#3399ff">
                        <span class="td_label">
                           <xsl:text>Sex</xsl:text>
                        </span>
                     </td>
                     <td>
                        <xsl:for-each select="n1:patient/n1:administrativeGenderCode">
                           <xsl:call-template name="show-gender"/>
                        </xsl:for-each>
                     </td>
                  </tr>
                  <xsl:if test="n1:patient/n1:raceCode | (n1:patient/n1:ethnicGroupCode)">
                     <tr>
                        <td width="20%" bgcolor="#3399ff">
                           <span class="td_label">
                              <xsl:text>Race</xsl:text>
                           </span>
                        </td>
                        <td width="30%">
                           <xsl:choose>
                              <xsl:when test="n1:patient/n1:raceCode">
                                 <xsl:for-each select="n1:patient/n1:raceCode">
                                    <xsl:call-template name="show-race-ethnicity"/>
                                 </xsl:for-each>
                              </xsl:when>
                              <xsl:otherwise>
                                 <xsl:text>Information not available</xsl:text>
                              </xsl:otherwise>
                           </xsl:choose>
                        </td>
                        <td width="15%" bgcolor="#3399ff">
                           <span class="td_label">
                              <xsl:text>Ethnicity</xsl:text>
                           </span>
                        </td>
                        <td>
                           <xsl:choose>
                              <xsl:when test="n1:patient/n1:ethnicGroupCode">
                                 <xsl:for-each select="n1:patient/n1:ethnicGroupCode">
                                    <xsl:call-template name="show-race-ethnicity"/>
                                 </xsl:for-each>
                              </xsl:when>
                              <xsl:otherwise>
                                 <xsl:text>Information not available</xsl:text>
                              </xsl:otherwise>
                           </xsl:choose>
                        </td>
                     </tr>
                  </xsl:if>
                  <tr>
                     <td bgcolor="#3399ff">
                        <span class="td_label">
                           <xsl:text>Contact info</xsl:text>
                        </span>
                     </td>
                     <td>
                        <xsl:call-template name="show-contactInfo">
                           <xsl:with-param name="contact" select="."/>
                        </xsl:call-template>
                     </td>
                     <td bgcolor="#3399ff">
                        <span class="td_label">
                           <xsl:text>Patient IDs</xsl:text>
                        </span>
                     </td>
                     <td>
                        <xsl:for-each select="n1:id">
                           <xsl:call-template name="show-id"/>
                           <br/>
                        </xsl:for-each>
                     </td>
                  </tr>
                  <xsl:for-each select="n1:patient/n1:languageCommunication[n1:preferenceInd[@value='true']]">
                     <tr>
                        <td width="20%" bgcolor="#3399ff">
                           <span class="td_label">
                              <xsl:text>Preferred Language</xsl:text>
                           </span>
                        </td>
                        <td colspan="3">
                           <xsl:apply-templates select="n1:languageCode" mode="ISO639LanguageISO3166Country"/>
                        </td>
                     </tr>
                  </xsl:for-each>
                  <xsl:for-each select="n1:patient/n1:languageCommunication[not(n1:preferenceInd/@value='true')]">
                     <tr>
                        <td width="20%" bgcolor="#3399ff">
                           <span class="td_label">
                              <xsl:text>Other Language</xsl:text>
                           </span>
                        </td>
                        <td colspan="3">
                           <xsl:apply-templates select="n1:languageCode" mode="ISO639LanguageISO3166Country"/>
                        </td>
                     </tr>
                  </xsl:for-each>
               </xsl:if>
            </xsl:for-each>
         </tbody>
      </table>
   </xsl:template>
   <!-- relatedDocument -->
   <xsl:template name="relatedDocument">
      <xsl:if test="n1:relatedDocument">
         <table class="header_table">
            <tbody>
               <xsl:for-each select="n1:relatedDocument">
                  <tr>
                     <td width="20%" bgcolor="#3399ff">
                        <span class="td_label">
                           <xsl:text>Related document</xsl:text>
                        </span>
                     </td>
                     <td width="80%">
                        <xsl:for-each select="n1:parentDocument">
                           <xsl:for-each select="n1:id">
                              <xsl:call-template name="show-id"/>
                              <br/>
                           </xsl:for-each>
                        </xsl:for-each>
                     </td>
                  </tr>
               </xsl:for-each>
            </tbody>
         </table>
      </xsl:if>
   </xsl:template>
   <!-- authorization (consent) -->
   <xsl:template name="authorization">
      <xsl:if test="n1:authorization">
         <table class="header_table">
            <tbody>
               <xsl:for-each select="n1:authorization">
                  <tr>
                     <td width="20%" bgcolor="#3399ff">
                        <span class="td_label">
                           <xsl:text>Consent</xsl:text>
                        </span>
                     </td>
                     <td width="80%">
                        <xsl:choose>
                           <xsl:when test="n1:consent/n1:code">
                              <xsl:call-template name="show-code">
                                 <xsl:with-param name="code" select="n1:consent/n1:code"/>
                              </xsl:call-template>
                           </xsl:when>
                           <xsl:otherwise>
                              <xsl:call-template name="show-code">
                                 <xsl:with-param name="code" select="n1:consent/n1:statusCode"/>
                              </xsl:call-template>
                           </xsl:otherwise>
                        </xsl:choose>
                        <br/>
                     </td>
                  </tr>
               </xsl:for-each>
            </tbody>
         </table>
      </xsl:if>
   </xsl:template>
   <!-- setAndVersion -->
   <xsl:template name="setAndVersion">
      <xsl:if test="n1:setId and n1:versionNumber">
         <table class="header_table">
            <tbody>
               <tr>
                  <td width="20%">
                     <xsl:text>SetId and Version</xsl:text>
                  </td>
                  <td colspan="3">
                     <xsl:text>SetId: </xsl:text>
                     <xsl:call-template name="show-id">
                        <xsl:with-param name="id" select="n1:setId"/>
                     </xsl:call-template>
                     <xsl:text>  Version: </xsl:text>
                     <xsl:value-of select="n1:versionNumber/@value"/>
                  </td>
               </tr>
            </tbody>
         </table>
      </xsl:if>
   </xsl:template>
   <!-- show StructuredBody  -->
   <xsl:template match="n1:component/n1:structuredBody">
      <xsl:for-each select="n1:component/n1:section">
         <xsl:call-template name="section"/>
      </xsl:for-each>
   </xsl:template>
   <!-- show nonXMLBody -->
   <xsl:template match='n1:component/n1:nonXMLBody'>
      <xsl:choose>
         <!-- if there is a reference, use that in an IFRAME -->
         <xsl:when test='n1:text/n1:reference'>
            <IFRAME name='nonXMLBody' id='nonXMLBody' WIDTH='80%' HEIGHT='600' src='{n1:text/n1:reference/@value}'/>
         </xsl:when>
         <xsl:when test='n1:text/@mediaType="text/plain"'>
            <pre>
               <xsl:value-of select='n1:text/text()'/>
            </pre>
         </xsl:when>
         <xsl:otherwise>
            <CENTER>Cannot display the text</CENTER>
         </xsl:otherwise>
      </xsl:choose>
   </xsl:template>
   <!-- top level component/section: display title and text,
     and process any nested component/sections
   -->
   <!-- (b.spivey - October 16th, 2013) - PLID 59066 -Give this section a title so I can hide it. -->
   <xsl:template name="section">
        <div id="{generate-id(n1:title)}" >
            <xsl:call-template name="section-title">
                <xsl:with-param name="title" select="n1:title"/>
            </xsl:call-template>
            <xsl:call-template name="section-author"/>
            <xsl:call-template name="section-text"/>
            <xsl:for-each select="n1:component/n1:section">
                <xsl:call-template name="nestedSection">
                    <xsl:with-param name="margin" select="2"/>
                </xsl:call-template>
            </xsl:for-each>
            <br />
        </div>
   </xsl:template>
   <!-- top level section title -->
   <xsl:template name="section-title">
      <xsl:param name="title"/>
      <xsl:choose>
         <xsl:when test="count(/n1:ClinicalDocument/n1:component/n1:structuredBody/n1:component[n1:section]) &gt; 1">
            <h3>
               <a name="{generate-id($title)}" href="#toc">
                  <xsl:value-of select="$title"/>
               </a>
            </h3>
         </xsl:when>
         <xsl:otherwise>
            <h3>
               <xsl:value-of select="$title"/>
            </h3>
         </xsl:otherwise>
      </xsl:choose>
   </xsl:template>
   <!-- section author -->
   <xsl:template name="section-author">
      <xsl:if test="count(n1:author)&gt;0">
         <div style="margin-left : 2em;">
            <b>
               <xsl:text>Section Author: </xsl:text>
            </b>
            <xsl:for-each select="n1:author/n1:assignedAuthor">
               <xsl:choose>
                  <xsl:when test="n1:assignedPerson/n1:name">
                     <xsl:call-template name="show-name">
                        <xsl:with-param name="name" select="n1:assignedPerson/n1:name"/>
                     </xsl:call-template>
                     <xsl:if test="n1:representedOrganization">
                        <xsl:text>, </xsl:text>
                        <xsl:call-template name="show-name">
                           <xsl:with-param name="name" select="n1:representedOrganization/n1:name"/>
                        </xsl:call-template>
                     </xsl:if>
                  </xsl:when>
                  <xsl:when test="n1:assignedAuthoringDevice/n1:softwareName">
                     <xsl:value-of select="n1:assignedAuthoringDevice/n1:softwareName"/>
                  </xsl:when>
                  <xsl:otherwise>
                     <xsl:for-each select="n1:id">
                        <xsl:call-template name="show-id"/>
                        <br/>
                     </xsl:for-each>
                  </xsl:otherwise>
               </xsl:choose>
            </xsl:for-each>
            <br/>
         </div>
      </xsl:if>
   </xsl:template>
   <!-- top-level section Text   -->
   <xsl:template name="section-text">
      <div>
         <xsl:apply-templates select="n1:text"/>
      </div>
   </xsl:template>
   <!-- nested component/section -->
   <xsl:template name="nestedSection">
      <xsl:param name="margin"/>
      <h4 style="margin-left : {$margin}em;">
         <xsl:value-of select="n1:title"/>
      </h4>
      <div style="margin-left : {$margin}em;">
         <xsl:apply-templates select="n1:text"/>
      </div>
      <xsl:for-each select="n1:component/n1:section">
         <xsl:call-template name="nestedSection">
            <xsl:with-param name="margin" select="2*$margin"/>
         </xsl:call-template>
      </xsl:for-each>
   </xsl:template>
   <!--   paragraph  -->
   <xsl:template match="n1:paragraph">
      <p>
         <xsl:apply-templates/>
      </p>
   </xsl:template>
   <!--   pre format  -->
   <xsl:template match="n1:pre">
      <pre>
         <xsl:apply-templates/>
      </pre>
   </xsl:template>
   <!--   Content w/ deleted text is hidden -->
   <xsl:template match="n1:content[@revised='delete']"/>
   <!--   content  -->
   <xsl:template match="n1:content">
      <xsl:apply-templates/>
   </xsl:template>
   <!-- line break -->
   <xsl:template match="n1:br">
      <xsl:element name='br'>
         <xsl:apply-templates/>
      </xsl:element>
   </xsl:template>
   <!--   list  -->
   <xsl:template match="n1:list">
      <xsl:if test="n1:caption">
         <p>
            <b>
               <xsl:apply-templates select="n1:caption"/>
            </b>
         </p>
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
         <span style="font-weight:bold; ">
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
   <!--  Tables   -->
   <xsl:template match="n1:table/@*|n1:thead/@*|n1:tfoot/@*|n1:tbody/@*|n1:colgroup/@*|n1:col/@*|n1:tr/@*|n1:th/@*|n1:td/@*">
      <xsl:copy>
         <xsl:copy-of select="@*"/>
         <xsl:apply-templates/>
      </xsl:copy>
   </xsl:template>
   <xsl:template match="n1:table">
      <table class="narr_table">
         <xsl:copy-of select="@*"/>
         <xsl:apply-templates/>
      </table>
   </xsl:template>
   <xsl:template match="n1:thead">
      <thead>
         <xsl:copy-of select="@*"/>
         <xsl:apply-templates/>
      </thead>
   </xsl:template>
   <xsl:template match="n1:tfoot">
      <tfoot>
         <xsl:copy-of select="@*"/>
         <xsl:apply-templates/>
      </tfoot>
   </xsl:template>
   <xsl:template match="n1:tbody">
      <tbody>
         <xsl:copy-of select="@*"/>
         <xsl:apply-templates/>
      </tbody>
   </xsl:template>
   <xsl:template match="n1:colgroup">
      <colgroup>
         <xsl:copy-of select="@*"/>
         <xsl:apply-templates/>
      </colgroup>
   </xsl:template>
   <xsl:template match="n1:col">
      <col>
         <xsl:copy-of select="@*"/>
         <xsl:apply-templates/>
      </col>
   </xsl:template>
   <xsl:template match="n1:tr">
      <tr class="narr_tr">
         <xsl:copy-of select="@*"/>
         <xsl:apply-templates/>
      </tr>
   </xsl:template>
   <xsl:template match="n1:th">
      <th class="narr_th">
         <xsl:copy-of select="@*"/>
         <xsl:apply-templates/>
      </th>
   </xsl:template>
   <xsl:template match="n1:td">
      <td>
         <xsl:copy-of select="@*"/>
         <xsl:apply-templates/>
      </td>
   </xsl:template>
   <xsl:template match="n1:table/n1:caption">
      <span style="font-weight:bold; ">
         <xsl:apply-templates/>
      </span>
   </xsl:template>
   <!--   RenderMultiMedia 
    this currently only handles GIF's and JPEG's.  It could, however,
    be extended by including other image MIME types in the predicate
    and/or by generating <object> or <applet> tag with the correct
    params depending on the media type  @ID  =$imageRef  referencedObject
    -->
   <xsl:template match="n1:renderMultiMedia">
      <xsl:variable name="imageRef" select="@referencedObject"/>
      <xsl:choose>
         <xsl:when test="//n1:regionOfInterest[@ID=$imageRef]">
            <!-- Here is where the Region of Interest image referencing goes -->
            <xsl:if test="//n1:regionOfInterest[@ID=$imageRef]//n1:observationMedia/n1:value[@mediaType='image/gif' or
 @mediaType='image/jpeg']">
               <br clear="all"/>
               <xsl:element name="img">
                  <xsl:attribute name="src"><xsl:value-of select="//n1:regionOfInterest[@ID=$imageRef]//n1:observationMedia/n1:value/n1:reference/@value"/></xsl:attribute>
               </xsl:element>
            </xsl:if>
         </xsl:when>
         <xsl:otherwise>
            <!-- Here is where the direct MultiMedia image referencing goes -->
            <xsl:if test="//n1:observationMedia[@ID=$imageRef]/n1:value[@mediaType='image/gif' or @mediaType='image/jpeg']">
               <br clear="all"/>
               <xsl:element name="img">
                  <xsl:attribute name="src"><xsl:value-of select="//n1:observationMedia[@ID=$imageRef]/n1:value/n1:reference/@value"/></xsl:attribute>
               </xsl:element>
            </xsl:if>
         </xsl:otherwise>
      </xsl:choose>
   </xsl:template>
   <!--    Stylecode processing   
    Supports Bold, Underline and Italics display
    -->
   <xsl:template match="//n1:*[@styleCode]">
      <xsl:if test="@styleCode='Bold'">
         <xsl:element name="b">
            <xsl:apply-templates/>
         </xsl:element>
      </xsl:if>
      <xsl:if test="@styleCode='Italics'">
         <xsl:element name="i">
            <xsl:apply-templates/>
         </xsl:element>
      </xsl:if>
      <xsl:if test="@styleCode='Underline'">
         <xsl:element name="u">
            <xsl:apply-templates/>
         </xsl:element>
      </xsl:if>
      <xsl:if test="contains(@styleCode,'Bold') and contains(@styleCode,'Italics') and not (contains(@styleCode, 'Underline'))">
         <xsl:element name="b">
            <xsl:element name="i">
               <xsl:apply-templates/>
            </xsl:element>
         </xsl:element>
      </xsl:if>
      <xsl:if test="contains(@styleCode,'Bold') and contains(@styleCode,'Underline') and not (contains(@styleCode, 'Italics'))">
         <xsl:element name="b">
            <xsl:element name="u">
               <xsl:apply-templates/>
            </xsl:element>
         </xsl:element>
      </xsl:if>
      <xsl:if test="contains(@styleCode,'Italics') and contains(@styleCode,'Underline') and not (contains(@styleCode, 'Bold'))">
         <xsl:element name="i">
            <xsl:element name="u">
               <xsl:apply-templates/>
            </xsl:element>
         </xsl:element>
      </xsl:if>
      <xsl:if test="contains(@styleCode,'Italics') and contains(@styleCode,'Underline') and contains(@styleCode, 'Bold')">
         <xsl:element name="b">
            <xsl:element name="i">
               <xsl:element name="u">
                  <xsl:apply-templates/>
               </xsl:element>
            </xsl:element>
         </xsl:element>
      </xsl:if>
      <xsl:if test="not (contains(@styleCode,'Italics') or contains(@styleCode,'Underline') or contains(@styleCode, 'Bold'))">
         <xsl:apply-templates/>
      </xsl:if>
   </xsl:template>
   <!--    Superscript or Subscript   -->
   <xsl:template match="n1:sup">
      <xsl:element name="sup">
         <xsl:apply-templates/>
      </xsl:element>
   </xsl:template>
   <xsl:template match="n1:sub">
      <xsl:element name="sub">
         <xsl:apply-templates/>
      </xsl:element>
   </xsl:template>
   <!-- show-signature -->
   <xsl:template name="show-sig">
      <xsl:param name="sig"/>
      <xsl:choose>
         <xsl:when test="$sig/@code =&apos;S&apos;">
            <xsl:text>signed</xsl:text>
         </xsl:when>
         <xsl:when test="$sig/@code=&apos;I&apos;">
            <xsl:text>intended</xsl:text>
         </xsl:when>
         <xsl:when test="$sig/@code=&apos;X&apos;">
            <xsl:text>signature required</xsl:text>
         </xsl:when>
      </xsl:choose>
   </xsl:template>
   <!--  show-id -->
   <xsl:template name="show-id">
      <xsl:param name="id"/>
      <xsl:choose>
         <xsl:when test="not($id)">
            <xsl:if test="not(@nullFlavor)">
               <xsl:if test="@extension">
                  <xsl:value-of select="@extension"/>
               </xsl:if>
               <xsl:text> </xsl:text>
               <xsl:value-of select="@root"/>
            </xsl:if>
         </xsl:when>
         <xsl:otherwise>
            <xsl:if test="not($id/@nullFlavor)">
               <xsl:if test="$id/@extension">
                  <xsl:value-of select="$id/@extension"/>
               </xsl:if>
               <xsl:text> </xsl:text>
               <xsl:value-of select="$id/@root"/>
            </xsl:if>
         </xsl:otherwise>
      </xsl:choose>
   </xsl:template>
   <!-- show-name  -->
   <xsl:template name="show-name">
      <xsl:param name="name"/>
      <xsl:choose>
         <xsl:when test="$name/n1:family">
            <xsl:if test="$name/n1:prefix">
               <xsl:value-of select="$name/n1:prefix"/>
               <xsl:text> </xsl:text>
            </xsl:if>
            <xsl:value-of select="$name/n1:given"/>
            <xsl:text> </xsl:text>
            <xsl:value-of select="$name/n1:family"/>
            <xsl:if test="$name/n1:suffix">
               <xsl:text>, </xsl:text>
               <xsl:value-of select="$name/n1:suffix"/>
            </xsl:if>
         </xsl:when>
         <xsl:otherwise>
            <xsl:value-of select="$name"/>
         </xsl:otherwise>
      </xsl:choose>
   </xsl:template>
   <!-- show-gender  -->
   <xsl:template name="show-gender">
      <xsl:choose>
         <xsl:when test="@code   = &apos;M&apos;">
            <xsl:text>Male</xsl:text>
         </xsl:when>
         <xsl:when test="@code  = &apos;F&apos;">
            <xsl:text>Female</xsl:text>
         </xsl:when>
         <xsl:when test="@code  = &apos;U&apos;">
            <xsl:text>Undifferentiated</xsl:text>
         </xsl:when>
      </xsl:choose>
   </xsl:template>
   <!-- show-race-ethnicity  -->
   <xsl:template name="show-race-ethnicity">
      <xsl:choose>
         <xsl:when test="@displayName">
            <xsl:value-of select="@displayName"/>
         </xsl:when>
         <xsl:otherwise>
            <xsl:value-of select="@code"/>
         </xsl:otherwise>
      </xsl:choose>
   </xsl:template>
   <!-- show-contactInfo -->
   <xsl:template name="show-contactInfo">
      <xsl:param name="contact"/>
      <xsl:call-template name="show-address">
         <xsl:with-param name="address" select="$contact/n1:addr"/>
      </xsl:call-template>
      <xsl:call-template name="show-telecom">
         <xsl:with-param name="telecom" select="$contact/n1:telecom"/>
      </xsl:call-template>
   </xsl:template>
   <!-- show-address -->
   <xsl:template name="show-address">
      <xsl:param name="address"/>
      <xsl:choose>
         <xsl:when test="$address">
            <xsl:if test="$address/@use">
               <xsl:text> </xsl:text>
               <xsl:call-template name="translateTelecomCode">
                  <xsl:with-param name="code" select="$address/@use"/>
               </xsl:call-template>
               <xsl:text>:</xsl:text>
               <br/>
            </xsl:if>
            <xsl:for-each select="$address/n1:streetAddressLine">
               <xsl:value-of select="."/>
               <br/>
            </xsl:for-each>
            <xsl:if test="$address/n1:streetName">
               <xsl:value-of select="$address/n1:streetName"/>
               <xsl:text> </xsl:text>
               <xsl:value-of select="$address/n1:houseNumber"/>
               <br/>
            </xsl:if>
            <xsl:if test="string-length($address/n1:city)>0">
               <xsl:value-of select="$address/n1:city"/>
            </xsl:if>
            <xsl:if test="string-length($address/n1:state)>0">
               <xsl:text>,&#160;</xsl:text>
               <xsl:value-of select="$address/n1:state"/>
            </xsl:if>
            <xsl:if test="string-length($address/n1:postalCode)>0">
               <xsl:text>&#160;</xsl:text>
               <xsl:value-of select="$address/n1:postalCode"/>
            </xsl:if>
            <xsl:if test="string-length($address/n1:country)>0">
               <xsl:text>,&#160;</xsl:text>
               <xsl:value-of select="$address/n1:country"/>
            </xsl:if>
         </xsl:when>
         <xsl:otherwise>
            <xsl:text>address not available</xsl:text>
         </xsl:otherwise>
      </xsl:choose>
      <br/>
   </xsl:template>
   <!-- show-telecom -->
   <xsl:template name="show-telecom">
      <xsl:param name="telecom"/>
      <xsl:choose>
         <xsl:when test="$telecom">
            <xsl:variable name="type" select="substring-before($telecom/@value, ':')"/>
            <xsl:variable name="value" select="substring-after($telecom/@value, ':')"/>
            <xsl:if test="$type">
               <xsl:call-template name="translateTelecomCode">
                  <xsl:with-param name="code" select="$type"/>
               </xsl:call-template>
               <xsl:if test="@use">
                  <xsl:text> (</xsl:text>
                  <xsl:call-template name="translateTelecomCode">
                     <xsl:with-param name="code" select="@use"/>
                  </xsl:call-template>
                  <xsl:text>)</xsl:text>
               </xsl:if>
               <xsl:text>: </xsl:text>
               <xsl:text> </xsl:text>
               <xsl:value-of select="$value"/>
            </xsl:if>
         </xsl:when>
         <xsl:otherwise>
            <xsl:text>Telecom information not available</xsl:text>
         </xsl:otherwise>
      </xsl:choose>
      <br/>
   </xsl:template>
   <!-- show-contactInfo -->
   <xsl:template name="show-contactInfoCompact">
      <xsl:param name="contact"/>
      <xsl:if test="$contact/n1:telecom | $contact/n1:addr">
        <xsl:text>- </xsl:text>
      </xsl:if>
      <xsl:call-template name="show-telecomCompact">
         <xsl:with-param name="telecom" select="$contact/n1:telecom"/>
      </xsl:call-template>
      <xsl:text>; </xsl:text>
      <xsl:call-template name="show-addressCompact">
         <xsl:with-param name="address" select="$contact/n1:addr"/>
      </xsl:call-template>
   </xsl:template>
   <!-- show-address -->
   <xsl:template name="show-addressCompact">
      <xsl:param name="address"/>
      <xsl:choose>
         <xsl:when test="$address">
            <xsl:if test="$address/@use">
               <xsl:text> </xsl:text>
               <xsl:call-template name="translateTelecomCode">
                  <xsl:with-param name="code" select="$address/@use"/>
               </xsl:call-template>
               <xsl:text>: </xsl:text>
            </xsl:if>
            <xsl:for-each select="$address/n1:streetAddressLine">
               <xsl:value-of select="."/>
               <xsl:text>, </xsl:text>
            </xsl:for-each>
            <xsl:if test="$address/n1:streetName">
               <xsl:value-of select="$address/n1:streetName"/>
               <xsl:text> </xsl:text>
               <xsl:value-of select="$address/n1:houseNumber"/>
               <xsl:text>, </xsl:text>,
            </xsl:if>
            <xsl:if test="string-length($address/n1:city)>0">
               <xsl:value-of select="$address/n1:city"/>
            </xsl:if>
            <xsl:if test="string-length($address/n1:state)>0">
               <xsl:text>,&#160;</xsl:text>
               <xsl:value-of select="$address/n1:state"/>
            </xsl:if>
            <xsl:if test="string-length($address/n1:postalCode)>0">
               <xsl:text>&#160;</xsl:text>
               <xsl:value-of select="$address/n1:postalCode"/>
            </xsl:if>
            <xsl:if test="string-length($address/n1:country)>0">
               <xsl:text>,&#160;</xsl:text>
               <xsl:value-of select="$address/n1:country"/>
            </xsl:if>
         </xsl:when>
      </xsl:choose>
   </xsl:template>
   <!-- show-telecom -->
   <xsl:template name="show-telecomCompact">
      <xsl:param name="telecom"/>
     <xsl:if test="$telecom/@value">
        <xsl:variable name="type" select="substring-before($telecom/@value, ':')"/>
        <xsl:variable name="value" select="substring-after($telecom/@value, ':')"/>
        <xsl:if test="$type">
           <xsl:call-template name="translateTelecomCode">
              <xsl:with-param name="code" select="$type"/>
           </xsl:call-template>
           <xsl:if test="@use">
              <xsl:text> (</xsl:text>
              <xsl:call-template name="translateTelecomCode">
                 <xsl:with-param name="code" select="@use"/>
              </xsl:call-template>
              <xsl:text>)</xsl:text>
           </xsl:if>
           <xsl:text>: </xsl:text>
           <xsl:text> </xsl:text>
        </xsl:if>
        <xsl:value-of select="$value"/>
     </xsl:if>
   </xsl:template>
   <!-- show-recipientType -->
   <xsl:template name="show-recipientType">
      <xsl:param name="typeCode"/>
      <xsl:choose>
         <xsl:when test="$typeCode='PRCP'">Primary Recipient:</xsl:when>
         <xsl:when test="$typeCode='TRC'">Secondary Recipient:</xsl:when>
         <xsl:otherwise>Recipient:</xsl:otherwise>
      </xsl:choose>
   </xsl:template>
   <!-- Convert Telecom URL to display text -->
   <xsl:template name="translateTelecomCode">
      <xsl:param name="code"/>
      <!--xsl:value-of select="document('voc.xml')/systems/system[@root=$code/@codeSystem]/code[@value=$code/@code]/@displayName"/-->
      <!--xsl:value-of select="document('codes.xml')/*/code[@code=$code]/@display"/-->
      <!-- This stuff needs to be transformed into uppercase values because xsl/xml is case sensitive -->
      <xsl:variable name="smallcase" select="'abcdefghijklmnopqrstuvwxyz'" />
      <xsl:variable name="uppercase" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'" />
      <xsl:variable name="codeTransform" select="translate($code, $smallcase, $uppercase)" />

      <xsl:choose>
      <!-- (b.spivey - October 16th, 2013) - PLID 59066 - case sensitive considerations.... -->
         <!-- lookup table Telecom URI -->
         <xsl:when test="$codeTransform='TEL'">
            <xsl:text>Tel</xsl:text>
         </xsl:when>
         <xsl:when test="$codeTransform='FAX'">
            <xsl:text>Fax</xsl:text>
         </xsl:when>
         <xsl:when test="$codeTransform='HTTP'">
            <xsl:text>Web</xsl:text>
         </xsl:when>
         <xsl:when test="$codeTransform='MAILTO'">
            <xsl:text>Mail</xsl:text>
         </xsl:when>
         <xsl:when test="$codeTransform='H'">
            <xsl:text>Home</xsl:text>
         </xsl:when>
         <xsl:when test="$codeTransform='HV'">
            <xsl:text>Vacation Home</xsl:text>
         </xsl:when>
         <xsl:when test="$codeTransform='HP'">
            <xsl:text>Primary Home</xsl:text>
         </xsl:when>
         <xsl:when test="$codeTransform='WP'">
            <xsl:text>Work Place</xsl:text>
         </xsl:when>
         <xsl:when test="$codeTransform='PUB'">
            <xsl:text>Pub</xsl:text>
         </xsl:when>
         <xsl:otherwise>
            <xsl:text>{$code='</xsl:text>
            <xsl:value-of select="$codeTransform"/>
            <xsl:text>'?}</xsl:text>
         </xsl:otherwise>
      </xsl:choose>
   </xsl:template>
   <!-- convert RoleClassAssociative code to display text -->
   <xsl:template name="translateRoleAssoCode">
      <xsl:param name="classCode"/>
      <xsl:param name="code"/>
      <xsl:choose>
         <xsl:when test="$classCode='AFFL'">
            <xsl:text>affiliate</xsl:text>
         </xsl:when>
         <xsl:when test="$classCode='AGNT'">
            <xsl:text>agent</xsl:text>
         </xsl:when>
         <xsl:when test="$classCode='ASSIGNED'">
            <xsl:text>assigned entity</xsl:text>
         </xsl:when>
         <xsl:when test="$classCode='COMPAR'">
            <xsl:text>commissioning party</xsl:text>
         </xsl:when>
         <xsl:when test="$classCode='CON'">
            <xsl:text>contact</xsl:text>
         </xsl:when>
         <xsl:when test="$classCode='ECON'">
            <xsl:text>emergency contact</xsl:text>
         </xsl:when>
         <xsl:when test="$classCode='NOK'">
            <xsl:text>next of kin</xsl:text>
         </xsl:when>
         <xsl:when test="$classCode='SGNOFF'">
            <xsl:text>signing authority</xsl:text>
         </xsl:when>
         <xsl:when test="$classCode='GUARD'">
            <xsl:text>guardian</xsl:text>
         </xsl:when>
         <xsl:when test="$classCode='GUAR'">
            <xsl:text>guardian</xsl:text>
         </xsl:when>
         <xsl:when test="$classCode='CIT'">
            <xsl:text>citizen</xsl:text>
         </xsl:when>
         <xsl:when test="$classCode='COVPTY'">
            <xsl:text>covered party</xsl:text>
         </xsl:when>
         <xsl:when test="$classCode='PRS'">
            <xsl:text>personal relationship</xsl:text>
         </xsl:when>
         <xsl:when test="$classCode='CAREGIVER'">
            <xsl:text>care giver</xsl:text>
         </xsl:when>
         <xsl:when test="$classCode='PROV'">
            <xsl:text>provider</xsl:text>
         </xsl:when>
         <xsl:otherwise>
            <xsl:text>{$classCode='</xsl:text>
            <xsl:value-of select="$classCode"/>
            <xsl:text>'?}</xsl:text>
         </xsl:otherwise>
      </xsl:choose>
      <xsl:if test="($code/@code) and ($code/@codeSystem='2.16.840.1.113883.5.111')">
         <xsl:text> </xsl:text>
         <xsl:choose>
            <xsl:when test="$code/@code='FTH'">
               <xsl:text>(Father)</xsl:text>
            </xsl:when>
            <xsl:when test="$code/@code='MTH'">
               <xsl:text>(Mother)</xsl:text>
            </xsl:when>
            <xsl:when test="$code/@code='NPRN'">
               <xsl:text>(Natural parent)</xsl:text>
            </xsl:when>
            <xsl:when test="$code/@code='STPPRN'">
               <xsl:text>(Step parent)</xsl:text>
            </xsl:when>
            <xsl:when test="$code/@code='SONC'">
               <xsl:text>(Son)</xsl:text>
            </xsl:when>
            <xsl:when test="$code/@code='DAUC'">
               <xsl:text>(Daughter)</xsl:text>
            </xsl:when>
            <xsl:when test="$code/@code='CHILD'">
               <xsl:text>(Child)</xsl:text>
            </xsl:when>
            <xsl:when test="$code/@code='EXT'">
               <xsl:text>(Extended family member)</xsl:text>
            </xsl:when>
            <xsl:when test="$code/@code='NBOR'">
               <xsl:text>(Neighbor)</xsl:text>
            </xsl:when>
            <xsl:when test="$code/@code='SIGOTHR'">
               <xsl:text>(Significant other)</xsl:text>
            </xsl:when>
            <xsl:otherwise>
               <xsl:text>{$code/@code='</xsl:text>
               <xsl:value-of select="$code/@code"/>
               <xsl:text>'?}</xsl:text>
            </xsl:otherwise>
         </xsl:choose>
      </xsl:if>
   </xsl:template>
   <!-- show time -->
   <xsl:template name="show-time">
      <xsl:param name="datetime"/>
      <xsl:choose>
         <xsl:when test="not($datetime)">
            <xsl:call-template name="formatDateTime">
               <xsl:with-param name="date" select="@value"/>
            </xsl:call-template>
            <xsl:text> </xsl:text>
         </xsl:when>
         <xsl:otherwise>
            <xsl:call-template name="formatDateTime">
               <xsl:with-param name="date" select="$datetime/@value"/>
            </xsl:call-template>
            <xsl:text> </xsl:text>
         </xsl:otherwise>
      </xsl:choose>
   </xsl:template>
   <!-- paticipant facility and date -->
   <xsl:template name="facilityAndDates">
      <table class="header_table">
         <tbody>
            <!-- facility id -->
            <tr>
               <td width="20%" bgcolor="#3399ff">
                  <span class="td_label">
                     <xsl:text>Facility ID</xsl:text>
                  </span>
               </td>
               <td colspan="3">
                  <xsl:choose>
                     <xsl:when test="count(/n1:ClinicalDocument/n1:participant
                                      [@typeCode='LOC'][@contextControlCode='OP']
                                      /n1:associatedEntity[@classCode='SDLOC']/n1:id)&gt;0">
                        <!-- change context node -->
                        <xsl:for-each select="/n1:ClinicalDocument/n1:participant
                                      [@typeCode='LOC'][@contextControlCode='OP']
                                      /n1:associatedEntity[@classCode='SDLOC']/n1:id">
                           <xsl:call-template name="show-id"/>
                           <!-- change context node again, for the code -->
                           <xsl:for-each select="../n1:code">
                              <xsl:text> (</xsl:text>
                              <xsl:call-template name="show-code">
                                 <xsl:with-param name="code" select="."/>
                              </xsl:call-template>
                              <xsl:text>)</xsl:text>
                           </xsl:for-each>
                        </xsl:for-each>
                     </xsl:when>
                     <xsl:otherwise>
                 Not available
                             </xsl:otherwise>
                  </xsl:choose>
               </td>
            </tr>
            <!-- Period reported -->
            <tr>
               <td width="20%" bgcolor="#3399ff">
                  <span class="td_label">
                     <xsl:text>First day of period reported</xsl:text>
                  </span>
               </td>
               <td colspan="3">
                  <xsl:call-template name="show-time">
                     <xsl:with-param name="datetime" select="/n1:ClinicalDocument/n1:documentationOf
                                      /n1:serviceEvent/n1:effectiveTime/n1:low"/>
                  </xsl:call-template>
               </td>
            </tr>
            <tr>
               <td width="20%" bgcolor="#3399ff">
                  <span class="td_label">
                     <xsl:text>Last day of period reported</xsl:text>
                  </span>
               </td>
               <td colspan="3">
                  <xsl:call-template name="show-time">
                     <xsl:with-param name="datetime" select="/n1:ClinicalDocument/n1:documentationOf
                                      /n1:serviceEvent/n1:effectiveTime/n1:high"/>
                  </xsl:call-template>
               </td>
            </tr>
         </tbody>
      </table>
   </xsl:template>
   <!-- show assignedEntity -->
   <xsl:template name="show-assignedEntity">
      <xsl:param name="asgnEntity"/>
      <xsl:choose>
         <xsl:when test="$asgnEntity/n1:assignedPerson/n1:name">
            <xsl:call-template name="show-name">
               <xsl:with-param name="name" select="$asgnEntity/n1:assignedPerson/n1:name"/>
            </xsl:call-template>
            <xsl:text> </xsl:text>
            <xsl:call-template name="show-contactInfoCompact">
                <xsl:with-param name="contact" select="$asgnEntity"/>
            </xsl:call-template>
            <xsl:text> </xsl:text>
            <xsl:if test="$asgnEntity/n1:representedOrganization/n1:name">
                <xsl:if test="$asgnEntity/n1:representedOrganization/n1:name != $asgnEntity/n1:assignedPerson/n1:name">
                    <xsl:text> (Organization: </xsl:text>
                    <xsl:call-template name="show-name">
                       <xsl:with-param name="name" select="$asgnEntity/n1:representedOrganization/n1:name"/>
                    </xsl:call-template>
                    <xsl:text> </xsl:text>
                    <xsl:call-template name="show-contactInfoCompact">
                        <xsl:with-param name="contact" select="$asgnEntity/n1:representedOrganization"/>
                    </xsl:call-template>
                    <xsl:text>) </xsl:text>
                </xsl:if>
            </xsl:if>
         </xsl:when>
         <xsl:when test="$asgnEntity/n1:representedOrganization">
            <xsl:call-template name="show-name">
               <xsl:with-param name="name" select="$asgnEntity/n1:representedOrganization/n1:name"/>
            </xsl:call-template>
            <xsl:call-template name="show-contactInfoCompact">
                <xsl:with-param name="contact" select="$asgnEntity/n1:representedOrganization"/>
            </xsl:call-template>
         </xsl:when>
         <xsl:otherwise>
            <xsl:for-each select="$asgnEntity/n1:id">
               <xsl:call-template name="show-id"/>
               <xsl:choose>
                  <xsl:when test="position()!=last()">
                     <xsl:text>, </xsl:text>
                  </xsl:when>
                  <xsl:otherwise>
                     <br/>
                  </xsl:otherwise>
               </xsl:choose>
            </xsl:for-each>
         </xsl:otherwise>
      </xsl:choose>
   </xsl:template>
   <!-- show relatedEntity -->
   <xsl:template name="show-relatedEntity">
      <xsl:param name="relatedEntity"/>
      <xsl:choose>
         <xsl:when test="$relatedEntity/n1:relatedPerson/n1:name">
            <xsl:call-template name="show-name">
               <xsl:with-param name="name" select="$relatedEntity/n1:relatedPerson/n1:name"/>
            </xsl:call-template>
         </xsl:when>
      </xsl:choose>
   </xsl:template>
   <!-- show associatedEntity -->
   <xsl:template name="show-associatedEntity">
      <xsl:param name="assoEntity"/>
      <xsl:choose>
         <xsl:when test="$assoEntity/n1:associatedPerson">
            <xsl:for-each select="$assoEntity/n1:associatedPerson/n1:name">
               <xsl:call-template name="show-name">
                  <xsl:with-param name="name" select="."/>
               </xsl:call-template>
               <br/>
            </xsl:for-each>
         </xsl:when>
         <xsl:when test="$assoEntity/n1:scopingOrganization">
            <xsl:for-each select="$assoEntity/n1:scopingOrganization">
               <xsl:if test="n1:name">
                  <xsl:call-template name="show-name">
                     <xsl:with-param name="name" select="n1:name"/>
                  </xsl:call-template>
                  <br/>
               </xsl:if>
               <xsl:if test="n1:standardIndustryClassCode">
                  <xsl:value-of select="n1:standardIndustryClassCode/@displayName"/>
                  <xsl:text> code:</xsl:text>
                  <xsl:value-of select="n1:standardIndustryClassCode/@code"/>
               </xsl:if>
            </xsl:for-each>
         </xsl:when>
         <xsl:when test="$assoEntity/n1:code">
            <xsl:call-template name="show-code">
               <xsl:with-param name="code" select="$assoEntity/n1:code"/>
            </xsl:call-template>
         </xsl:when>
         <xsl:when test="$assoEntity/n1:id">
            <xsl:value-of select="$assoEntity/n1:id/@extension"/>
            <xsl:text> </xsl:text>
            <xsl:value-of select="$assoEntity/n1:id/@root"/>
         </xsl:when>
      </xsl:choose>
   </xsl:template>
   <!-- show code 
    if originalText present, return it, otherwise, check and return attribute: display name
    -->
   <xsl:template name="show-code">
      <xsl:param name="code"/>
      <xsl:variable name="this-codeSystem">
         <xsl:value-of select="$code/@codeSystem"/>
      </xsl:variable>
      <xsl:variable name="this-code">
         <xsl:value-of select="$code/@code"/>
      </xsl:variable>
      <xsl:choose>
         <xsl:when test="$code/n1:originalText">
            <xsl:value-of select="$code/n1:originalText"/>
         </xsl:when>
         <xsl:when test="$code/@displayName">
            <xsl:value-of select="$code/@displayName"/>
         </xsl:when>
         <!--
      <xsl:when test="$the-valuesets/*/voc:system[@root=$this-codeSystem]/voc:code[@value=$this-code]/@displayName">
        <xsl:value-of select="$the-valuesets/*/voc:system[@root=$this-codeSystem]/voc:code[@value=$this-code]/@displayName"/>
      </xsl:when>
      -->
         <xsl:otherwise>
            <xsl:value-of select="$this-code"/>
         </xsl:otherwise>
      </xsl:choose>
   </xsl:template>
   <!-- show classCode -->
   <xsl:template name="show-actClassCode">
      <xsl:param name="clsCode"/>
      <xsl:choose>
         <xsl:when test=" $clsCode = 'ACT' ">
            <xsl:text>healthcare service</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'ACCM' ">
            <xsl:text>accommodation</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'ACCT' ">
            <xsl:text>account</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'ACSN' ">
            <xsl:text>accession</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'ADJUD' ">
            <xsl:text>financial adjudication</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'CONS' ">
            <xsl:text>consent</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'CONTREG' ">
            <xsl:text>container registration</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'CTTEVENT' ">
            <xsl:text>clinical trial timepoint event</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'DISPACT' ">
            <xsl:text>disciplinary action</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'ENC' ">
            <xsl:text>encounter</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'INC' ">
            <xsl:text>incident</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'INFRM' ">
            <xsl:text>inform</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'INVE' ">
            <xsl:text>invoice element</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'LIST' ">
            <xsl:text>working list</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'MPROT' ">
            <xsl:text>monitoring program</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'PCPR' ">
            <xsl:text>care provision</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'PROC' ">
            <xsl:text>procedure</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'REG' ">
            <xsl:text>registration</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'REV' ">
            <xsl:text>review</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'SBADM' ">
            <xsl:text>substance administration</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'SPCTRT' ">
            <xsl:text>speciment treatment</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'SUBST' ">
            <xsl:text>substitution</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'TRNS' ">
            <xsl:text>transportation</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'VERIF' ">
            <xsl:text>verification</xsl:text>
         </xsl:when>
         <xsl:when test=" $clsCode = 'XACT' ">
            <xsl:text>financial transaction</xsl:text>
         </xsl:when>
      </xsl:choose>
   </xsl:template>
   <!-- show participationType -->
   <xsl:template name="show-participationType">
      <xsl:param name="ptype"/>
      <xsl:choose>
         <xsl:when test=" $ptype='PPRF' ">
            <xsl:text>primary performer</xsl:text>
         </xsl:when>
         <xsl:when test=" $ptype='PRF' ">
            <xsl:text>performer</xsl:text>
         </xsl:when>
         <xsl:when test=" $ptype='VRF' ">
            <xsl:text>verifier</xsl:text>
         </xsl:when>
         <xsl:when test=" $ptype='SPRF' ">
            <xsl:text>secondary performer</xsl:text>
         </xsl:when>
      </xsl:choose>
   </xsl:template>
   <!-- show participationFunction -->
   <xsl:template name="show-participationFunction">
      <xsl:param name="pFunction"/>
      <xsl:choose>
         <!-- From the HL7 v3 ParticipationFunction code system -->
         <xsl:when test=" $pFunction = 'ADMPHYS' ">
            <xsl:text>(admitting physician)</xsl:text>
         </xsl:when>
         <xsl:when test=" $pFunction = 'ANEST' ">
            <xsl:text>(anesthesist)</xsl:text>
         </xsl:when>
         <xsl:when test=" $pFunction = 'ANRS' ">
            <xsl:text>(anesthesia nurse)</xsl:text>
         </xsl:when>
         <xsl:when test=" $pFunction = 'ATTPHYS' ">
            <xsl:text>(attending physician)</xsl:text>
         </xsl:when>
         <xsl:when test=" $pFunction = 'DISPHYS' ">
            <xsl:text>(discharging physician)</xsl:text>
         </xsl:when>
         <xsl:when test=" $pFunction = 'FASST' ">
            <xsl:text>(first assistant surgeon)</xsl:text>
         </xsl:when>
         <xsl:when test=" $pFunction = 'MDWF' ">
            <xsl:text>(midwife)</xsl:text>
         </xsl:when>
         <xsl:when test=" $pFunction = 'NASST' ">
            <xsl:text>(nurse assistant)</xsl:text>
         </xsl:when>
         <xsl:when test=" $pFunction = 'PCP' ">
            <xsl:text>(primary care physician)</xsl:text>
         </xsl:when>
         <xsl:when test=" $pFunction = 'PRISURG' ">
            <xsl:text>(primary surgeon)</xsl:text>
         </xsl:when>
         <xsl:when test=" $pFunction = 'RNDPHYS' ">
            <xsl:text>(rounding physician)</xsl:text>
         </xsl:when>
         <xsl:when test=" $pFunction = 'SASST' ">
            <xsl:text>(second assistant surgeon)</xsl:text>
         </xsl:when>
         <xsl:when test=" $pFunction = 'SNRS' ">
            <xsl:text>(scrub nurse)</xsl:text>
         </xsl:when>
         <xsl:when test=" $pFunction = 'TASST' ">
            <xsl:text>(third assistant)</xsl:text>
         </xsl:when>
         <!-- From the HL7 v2 Provider Role code system (2.16.840.1.113883.12.443) which is used by HITSP -->
         <xsl:when test=" $pFunction = 'CP' ">
            <xsl:text>(consulting provider)</xsl:text>
         </xsl:when>
         <xsl:when test=" $pFunction = 'PP' ">
            <xsl:text>(primary care provider)</xsl:text>
         </xsl:when>
         <xsl:when test=" $pFunction = 'RP' ">
            <xsl:text>(referring provider)</xsl:text>
         </xsl:when>
         <xsl:when test=" $pFunction = 'MP' ">
            <xsl:text>(medical home provider)</xsl:text>
         </xsl:when>
      </xsl:choose>
   </xsl:template>
   <xsl:template name="formatDateTime">
      <xsl:param name="date"/>
      <!-- month -->
      <xsl:variable name="month" select="substring ($date, 5, 2)"/>
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
      <!-- day -->
      <xsl:choose>
         <xsl:when test='substring ($date, 7, 1)="0"'>
            <xsl:value-of select="substring ($date, 8, 1)"/>
            <xsl:text>, </xsl:text>
         </xsl:when>
         <xsl:otherwise>
            <xsl:value-of select="substring ($date, 7, 2)"/>
            <xsl:text>, </xsl:text>
         </xsl:otherwise>
      </xsl:choose>
      <!-- year -->
      <xsl:value-of select="substring ($date, 1, 4)"/>
      <!-- time and US timezone -->
      <xsl:if test="string-length($date) > 8">
         <xsl:text>, </xsl:text>
         <!-- time -->
         <xsl:variable name="time">
            <xsl:value-of select="substring($date,9,6)"/>
         </xsl:variable>
         <xsl:variable name="hh">
            <xsl:value-of select="substring($time,1,2)"/>
         </xsl:variable>
         <xsl:variable name="mm">
            <xsl:value-of select="substring($time,3,2)"/>
         </xsl:variable>
         <xsl:variable name="ss">
            <xsl:value-of select="substring($time,5,2)"/>
         </xsl:variable>
         <xsl:if test="string-length($hh)&gt;1">
            <xsl:value-of select="$hh"/>
            <xsl:if test="string-length($mm)&gt;1 and not(contains($mm,'-')) and not (contains($mm,'+'))">
               <xsl:text>:</xsl:text>
               <xsl:value-of select="$mm"/>
               <xsl:if test="string-length($ss)&gt;1 and not(contains($ss,'-')) and not (contains($ss,'+'))">
                  <xsl:text>:</xsl:text>
                  <xsl:value-of select="$ss"/>
               </xsl:if>
            </xsl:if>
         </xsl:if>
         <!-- time zone -->
         <xsl:variable name="tzon">
            <xsl:choose>
               <xsl:when test="contains($date,'+')">
                  <xsl:text>+</xsl:text>
                  <xsl:value-of select="substring-after($date, '+')"/>
               </xsl:when>
               <xsl:when test="contains($date,'-')">
                  <xsl:text>-</xsl:text>
                  <xsl:value-of select="substring-after($date, '-')"/>
               </xsl:when>
            </xsl:choose>
         </xsl:variable>
         <xsl:choose>
            <!-- reference: http://www.timeanddate.com/library/abbreviations/timezones/na/ -->
            <xsl:when test="$tzon = '-0500' ">
               <xsl:text>, EST</xsl:text>
            </xsl:when>
            <xsl:when test="$tzon = '-0600' ">
               <xsl:text>, CST</xsl:text>
            </xsl:when>
            <xsl:when test="$tzon = '-0700' ">
               <xsl:text>, MST</xsl:text>
            </xsl:when>
            <xsl:when test="$tzon = '-0800' ">
               <xsl:text>, PST</xsl:text>
            </xsl:when>
            <xsl:otherwise>
               <xsl:text> </xsl:text>
               <xsl:value-of select="$tzon"/>
            </xsl:otherwise>
         </xsl:choose>
      </xsl:if>
   </xsl:template>
   <!-- convert to lower case -->
   <xsl:template name="caseDown">
      <xsl:param name="data"/>
      <xsl:if test="$data">
         <xsl:value-of select="translate($data, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ','abcdefghijklmnopqrstuvwxyz')"/>
      </xsl:if>
   </xsl:template>
   <!-- convert to upper case -->
   <xsl:template name="caseUp">
      <xsl:param name="data"/>
      <xsl:if test="$data">
         <xsl:value-of select="translate($data,'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ')"/>
      </xsl:if>
   </xsl:template>
   <!-- convert first character to upper case -->
   <xsl:template name="firstCharCaseUp">
      <xsl:param name="data"/>
      <xsl:if test="$data">
         <xsl:call-template name="caseUp">
            <xsl:with-param name="data" select="substring($data,1,1)"/>
         </xsl:call-template>
         <xsl:value-of select="substring($data,2)"/>
      </xsl:if>
   </xsl:template>
   <!-- show-noneFlavor -->
   <xsl:template name="show-noneFlavor">
      <xsl:param name="nf"/>
      <xsl:choose>
         <xsl:when test=" $nf = 'NI' ">
            <xsl:text>no information</xsl:text>
         </xsl:when>
         <xsl:when test=" $nf = 'INV' ">
            <xsl:text>invalid</xsl:text>
         </xsl:when>
         <xsl:when test=" $nf = 'MSK' ">
            <xsl:text>masked</xsl:text>
         </xsl:when>
         <xsl:when test=" $nf = 'NA' ">
            <xsl:text>not applicable</xsl:text>
         </xsl:when>
         <xsl:when test=" $nf = 'UNK' ">
            <xsl:text>unknown</xsl:text>
         </xsl:when>
         <xsl:when test=" $nf = 'OTH' ">
            <xsl:text>other</xsl:text>
         </xsl:when>
      </xsl:choose>
   </xsl:template>
    <xsl:template name="show-languageName">
      <xsl:param name='languageCode'/>
<xsl:variable name="languageCodes">
<lang code3="aar" code2="aa">Afar</lang>
<lang code3="abk" code2="ab">Abkhazian</lang>
<lang code3="ace">Achinese</lang>
<lang code3="ach">Acoli</lang>
<lang code3="ada">Adangme</lang>
<lang code3="ady">Adyghe; Adygei</lang>
<lang code3="afa">Afro-Asiatic languages</lang>
<lang code3="afh">Afrihili</lang>
<lang code3="afr" code2="af">Afrikaans</lang>
<lang code3="ain">Ainu</lang>
<lang code3="aka" code2="ak">Akan</lang>
<lang code3="akk">Akkadian</lang>
<lang code3="alb" code2="sq">Albanian</lang>
<lang code3="ale">Aleut</lang>
<lang code3="alg">Algonquian languages</lang>
<lang code3="alt">Southern Altai</lang>
<lang code3="amh" code2="am">Amharic</lang>
<lang code3="ang">English, Old (ca.450-1100)</lang>
<lang code3="anp">Angika</lang>
<lang code3="apa">Apache languages</lang>
<lang code3="ara" code2="ar">Arabic</lang>
<lang code3="arc">Official Aramaic (700-300 BCE); Imperial Aramaic (700-300 BCE)</lang>
<lang code3="arg" code2="an">Aragonese</lang>
<lang code3="arm" code2="hy">Armenian</lang>
<lang code3="arn">Mapudungun; Mapuche</lang>
<lang code3="arp">Arapaho</lang>
<lang code3="art">Artificial languages</lang>
<lang code3="arw">Arawak</lang>
<lang code3="asm" code2="as">Assamese</lang>
<lang code3="ast">Asturian; Bable; Leonese; Asturleonese</lang>
<lang code3="ath">Athapascan languages</lang>
<lang code3="aus">Australian languages</lang>
<lang code3="ava" code2="av">Avaric</lang>
<lang code3="ave" code2="ae">Avestan</lang>
<lang code3="awa">Awadhi</lang>
<lang code3="aym" code2="ay">Aymara</lang>
<lang code3="aze" code2="az">Azerbaijani</lang>
<lang code3="bad">Banda languages</lang>
<lang code3="bai">Bamileke languages</lang>
<lang code3="bak" code2="ba">Bashkir</lang>
<lang code3="bal">Baluchi</lang>
<lang code3="bam" code2="bm">Bambara</lang>
<lang code3="ban">Balinese</lang>
<lang code3="baq" code2="eu">Basque</lang>
<lang code3="bas">Basa</lang>
<lang code3="bat">Baltic languages</lang>
<lang code3="bej">Beja; Bedawiyet</lang>
<lang code3="bel" code2="be">Belarusian</lang>
<lang code3="bem">Bemba</lang>
<lang code3="ben" code2="bn">Bengali</lang>
<lang code3="ber">Berber languages</lang>
<lang code3="bho">Bhojpuri</lang>
<lang code3="bih" code2="bh">Bihari languages</lang>
<lang code3="bik">Bikol</lang>
<lang code3="bin">Bini; Edo</lang>
<lang code3="bis" code2="bi">Bislama</lang>
<lang code3="bla">Siksika</lang>
<lang code3="bnt">Bantu languages</lang>
<lang code3="bos" code2="bs">Bosnian</lang>
<lang code3="bra">Braj</lang>
<lang code3="bre" code2="br">Breton</lang>
<lang code3="btk">Batak languages</lang>
<lang code3="bua">Buriat</lang>
<lang code3="bug">Buginese</lang>
<lang code3="bul" code2="bg">Bulgarian</lang>
<lang code3="bur" code2="my">Burmese</lang>
<lang code3="byn">Blin; Bilin</lang>
<lang code3="cad">Caddo</lang>
<lang code3="cai">Central American Indian languages</lang>
<lang code3="car">Galibi Carib</lang>
<lang code3="cat" code2="ca">Catalan; Valencian</lang>
<lang code3="cau">Caucasian languages</lang>
<lang code3="ceb">Cebuano</lang>
<lang code3="cel">Celtic languages</lang>
<lang code3="cha" code2="ch">Chamorro</lang>
<lang code3="chb">Chibcha</lang>
<lang code3="che" code2="ce">Chechen</lang>
<lang code3="chg">Chagatai</lang>
<lang code3="chi" code2="zh">Chinese</lang>
<lang code3="chk">Chuukese</lang>
<lang code3="chm">Mari</lang>
<lang code3="chn">Chinook jargon</lang>
<lang code3="cho">Choctaw</lang>
<lang code3="chp">Chipewyan; Dene Suline</lang>
<lang code3="chr">Cherokee</lang>
<lang code3="chu" code2="cu">Church Slavic; Old Slavonic; Church Slavonic; Old Bulgarian; Old Church Slavonic</lang>
<lang code3="chv" code2="cv">Chuvash</lang>
<lang code3="chy">Cheyenne</lang>
<lang code3="cmc">Chamic languages</lang>
<lang code3="cop">Coptic</lang>
<lang code3="cor" code2="kw">Cornish</lang>
<lang code3="cos" code2="co">Corsican</lang>
<lang code3="cpe">Creoles and pidgins, English based</lang>
<lang code3="cpf">Creoles and pidgins, French-based</lang>
<lang code3="cpp">Creoles and pidgins, Portuguese-based</lang>
<lang code3="cre" code2="cr">Cree</lang>
<lang code3="crh">Crimean Tatar; Crimean Turkish</lang>
<lang code3="crp">Creoles and pidgins</lang>
<lang code3="csb">Kashubian</lang>
<lang code3="cus">Cushitic languages</lang>
<lang code3="cze" code2="cs">Czech</lang>
<lang code3="dak">Dakota</lang>
<lang code3="dan" code2="da">Danish</lang>
<lang code3="dar">Dargwa</lang>
<lang code3="day">Land Dayak languages</lang>
<lang code3="del">Delaware</lang>
<lang code3="den">Slave (Athapascan)</lang>
<lang code3="dgr">Dogrib</lang>
<lang code3="din">Dinka</lang>
<lang code3="div" code2="dv">Divehi; Dhivehi; Maldivian</lang>
<lang code3="doi">Dogri</lang>
<lang code3="dra">Dravidian languages</lang>
<lang code3="dsb">Lower Sorbian</lang>
<lang code3="dua">Duala</lang>
<lang code3="dum">Dutch, Middle (ca.1050-1350)</lang>
<lang code3="dut" code2="nl">Dutch; Flemish</lang>
<lang code3="dyu">Dyula</lang>
<lang code3="dzo" code2="dz">Dzongkha</lang>
<lang code3="efi">Efik</lang>
<lang code3="egy">Egyptian (Ancient)</lang>
<lang code3="eka">Ekajuk</lang>
<lang code3="elx">Elamite</lang>
<lang code3="eng" code2="en">English</lang>
<lang code3="enm">English, Middle (1100-1500)</lang>
<lang code3="epo" code2="eo">Esperanto</lang>
<lang code3="est" code2="et">Estonian</lang>
<lang code3="ewe" code2="ee">Ewe</lang>
<lang code3="ewo">Ewondo</lang>
<lang code3="fan">Fang</lang>
<lang code3="fao" code2="fo">Faroese</lang>
<lang code3="fat">Fanti</lang>
<lang code3="fij" code2="fj">Fijian</lang>
<lang code3="fil">Filipino; Pilipino</lang>
<lang code3="fin" code2="fi">Finnish</lang>
<lang code3="fiu">Finno-Ugrian languages</lang>
<lang code3="fon">Fon</lang>
<lang code3="fre" code2="fr">French</lang>
<lang code3="frm">French, Middle (ca.1400-1600)</lang>
<lang code3="fro">French, Old (842-ca.1400)</lang>
<lang code3="frr">Northern Frisian</lang>
<lang code3="frs">Eastern Frisian</lang>
<lang code3="fry" code2="fy">Western Frisian</lang>
<lang code3="ful" code2="ff">Fulah</lang>
<lang code3="fur">Friulian</lang>
<lang code3="gaa">Ga</lang>
<lang code3="gay">Gayo</lang>
<lang code3="gba">Gbaya</lang>
<lang code3="gem">Germanic languages</lang>
<lang code3="geo" code2="ka">Georgian</lang>
<lang code3="ger" code2="de">German</lang>
<lang code3="gez">Geez</lang>
<lang code3="gil">Gilbertese</lang>
<lang code3="gla" code2="gd">Gaelic; Scottish Gaelic</lang>
<lang code3="gle" code2="ga">Irish</lang>
<lang code3="glg" code2="gl">Galician</lang>
<lang code3="glv" code2="gv">Manx</lang>
<lang code3="gmh">German, Middle High (ca.1050-1500)</lang>
<lang code3="goh">German, Old High (ca.750-1050)</lang>
<lang code3="gon">Gondi</lang>
<lang code3="gor">Gorontalo</lang>
<lang code3="got">Gothic</lang>
<lang code3="grb">Grebo</lang>
<lang code3="grc">Greek, Ancient (to 1453)</lang>
<lang code3="gre" code2="el">Greek, Modern (1453-)</lang>
<lang code3="grn" code2="gn">Guarani</lang>
<lang code3="gsw">Swiss German; Alemannic; Alsatian</lang>
<lang code3="guj" code2="gu">Gujarati</lang>
<lang code3="gwi">Gwich'in</lang>
<lang code3="hai">Haida</lang>
<lang code3="hat" code2="ht">Haitian; Haitian Creole</lang>
<lang code3="hau" code2="ha">Hausa</lang>
<lang code3="haw">Hawaiian</lang>
<lang code3="heb" code2="he">Hebrew</lang>
<lang code3="her" code2="hz">Herero</lang>
<lang code3="hil">Hiligaynon</lang>
<lang code3="him">Himachali languages; Western Pahari languages</lang>
<lang code3="hin" code2="hi">Hindi</lang>
<lang code3="hit">Hittite</lang>
<lang code3="hmn">Hmong; Mong</lang>
<lang code3="hmo" code2="ho">Hiri Motu</lang>
<lang code3="hrv" code2="hr">Croatian</lang>
<lang code3="hsb">Upper Sorbian</lang>
<lang code3="hun" code2="hu">Hungarian</lang>
<lang code3="hup">Hupa</lang>
<lang code3="iba">Iban</lang>
<lang code3="ibo" code2="ig">Igbo</lang>
<lang code3="ice" code2="is">Icelandic</lang>
<lang code3="ido" code2="io">Ido</lang>
<lang code3="iii" code2="ii">Sichuan Yi; Nuosu</lang>
<lang code3="ijo">Ijo languages</lang>
<lang code3="iku" code2="iu">Inuktitut</lang>
<lang code3="ile" code2="ie">Interlingue; Occidental</lang>
<lang code3="ilo">Iloko</lang>
<lang code3="ina" code2="ia">Interlingua (International Auxiliary Language Association)</lang>
<lang code3="inc">Indic languages</lang>
<lang code3="ind" code2="id">Indonesian</lang>
<lang code3="ine">Indo-European languages</lang>
<lang code3="inh">Ingush</lang>
<lang code3="ipk" code2="ik">Inupiaq</lang>
<lang code3="ira">Iranian languages</lang>
<lang code3="iro">Iroquoian languages</lang>
<lang code3="ita" code2="it">Italian</lang>
<lang code3="jav" code2="jv">Javanese</lang>
<lang code3="jbo">Lojban</lang>
<lang code3="jpn" code2="ja">Japanese</lang>
<lang code3="jpr">Judeo-Persian</lang>
<lang code3="jrb">Judeo-Arabic</lang>
<lang code3="kaa">Kara-Kalpak</lang>
<lang code3="kab">Kabyle</lang>
<lang code3="kac">Kachin; Jingpho</lang>
<lang code3="kal" code2="kl">Kalaallisut; Greenlandic</lang>
<lang code3="kam">Kamba</lang>
<lang code3="kan" code2="kn">Kannada</lang>
<lang code3="kar">Karen languages</lang>
<lang code3="kas" code2="ks">Kashmiri</lang>
<lang code3="kau" code2="kr">Kanuri</lang>
<lang code3="kaw">Kawi</lang>
<lang code3="kaz" code2="kk">Kazakh</lang>
<lang code3="kbd">Kabardian</lang>
<lang code3="kha">Khasi</lang>
<lang code3="khi">Khoisan languages</lang>
<lang code3="khm" code2="km">Central Khmer</lang>
<lang code3="kho">Khotanese; Sakan</lang>
<lang code3="kik" code2="ki">Kikuyu; Gikuyu</lang>
<lang code3="kin" code2="rw">Kinyarwanda</lang>
<lang code3="kir" code2="ky">Kirghiz; Kyrgyz</lang>
<lang code3="kmb">Kimbundu</lang>
<lang code3="kok">Konkani</lang>
<lang code3="kom" code2="kv">Komi</lang>
<lang code3="kon" code2="kg">Kongo</lang>
<lang code3="kor" code2="ko">Korean</lang>
<lang code3="kos">Kosraean</lang>
<lang code3="kpe">Kpelle</lang>
<lang code3="krc">Karachay-Balkar</lang>
<lang code3="krl">Karelian</lang>
<lang code3="kro">Kru languages</lang>
<lang code3="kru">Kurukh</lang>
<lang code3="kua" code2="kj">Kuanyama; Kwanyama</lang>
<lang code3="kum">Kumyk</lang>
<lang code3="kur" code2="ku">Kurdish</lang>
<lang code3="kut">Kutenai</lang>
<lang code3="lad">Ladino</lang>
<lang code3="lah">Lahnda</lang>
<lang code3="lam">Lamba</lang>
<lang code3="lao" code2="lo">Lao</lang>
<lang code3="lat" code2="la">Latin</lang>
<lang code3="lav" code2="lv">Latvian</lang>
<lang code3="lez">Lezghian</lang>
<lang code3="lim" code2="li">Limburgan; Limburger; Limburgish</lang>
<lang code3="lin" code2="ln">Lingala</lang>
<lang code3="lit" code2="lt">Lithuanian</lang>
<lang code3="lol">Mongo</lang>
<lang code3="loz">Lozi</lang>
<lang code3="ltz" code2="lb">Luxembourgish; Letzeburgesch</lang>
<lang code3="lua">Luba-Lulua</lang>
<lang code3="lub" code2="lu">Luba-Katanga</lang>
<lang code3="lug" code2="lg">Ganda</lang>
<lang code3="lui">Luiseno</lang>
<lang code3="lun">Lunda</lang>
<lang code3="luo">Luo (Kenya and Tanzania)</lang>
<lang code3="lus">Lushai</lang>
<lang code3="mac" code2="mk">Macedonian</lang>
<lang code3="mad">Madurese</lang>
<lang code3="mag">Magahi</lang>
<lang code3="mah" code2="mh">Marshallese</lang>
<lang code3="mai">Maithili</lang>
<lang code3="mak">Makasar</lang>
<lang code3="mal" code2="ml">Malayalam</lang>
<lang code3="man">Mandingo</lang>
<lang code3="mao" code2="mi">Maori</lang>
<lang code3="map">Austronesian languages</lang>
<lang code3="mar" code2="mr">Marathi</lang>
<lang code3="mas">Masai</lang>
<lang code3="may" code2="ms">Malay</lang>
<lang code3="mdf">Moksha</lang>
<lang code3="mdr">Mandar</lang>
<lang code3="men">Mende</lang>
<lang code3="mga">Irish, Middle (900-1200)</lang>
<lang code3="mic">Mi'kmaq; Micmac</lang>
<lang code3="min">Minangkabau</lang>
<lang code3="mis">Uncoded languages</lang>
<lang code3="mkh">Mon-Khmer languages</lang>
<lang code3="mlg" code2="mg">Malagasy</lang>
<lang code3="mlt" code2="mt">Maltese</lang>
<lang code3="mnc">Manchu</lang>
<lang code3="mni">Manipuri</lang>
<lang code3="mno">Manobo languages</lang>
<lang code3="moh">Mohawk</lang>
<lang code3="mon" code2="mn">Mongolian</lang>
<lang code3="mos">Mossi</lang>
<lang code3="mul">Multiple languages</lang>
<lang code3="mun">Munda languages</lang>
<lang code3="mus">Creek</lang>
<lang code3="mwl">Mirandese</lang>
<lang code3="mwr">Marwari</lang>
<lang code3="myn">Mayan languages</lang>
<lang code3="myv">Erzya</lang>
<lang code3="nah">Nahuatl languages</lang>
<lang code3="nai">North American Indian languages</lang>
<lang code3="nap">Neapolitan</lang>
<lang code3="nau" code2="na">Nauru</lang>
<lang code3="nav" code2="nv">Navajo; Navaho</lang>
<lang code3="nbl" code2="nr">Ndebele, South; South Ndebele</lang>
<lang code3="nde" code2="nd">Ndebele, North; North Ndebele</lang>
<lang code3="ndo" code2="ng">Ndonga</lang>
<lang code3="nds">Low German; Low Saxon; German, Low; Saxon, Low</lang>
<lang code3="nep" code2="ne">Nepali</lang>
<lang code3="new">Nepal Bhasa; Newari</lang>
<lang code3="nia">Nias</lang>
<lang code3="nic">Niger-Kordofanian languages</lang>
<lang code3="niu">Niuean</lang>
<lang code3="nno" code2="nn">Norwegian Nynorsk; Nynorsk, Norwegian</lang>
<lang code3="nob" code2="nb">Bokmål, Norwegian; Norwegian Bokmål</lang>
<lang code3="nog">Nogai</lang>
<lang code3="non">Norse, Old</lang>
<lang code3="nor" code2="no">Norwegian</lang>
<lang code3="nqo">N'Ko</lang>
<lang code3="nso">Pedi; Sepedi; Northern Sotho</lang>
<lang code3="nub">Nubian languages</lang>
<lang code3="nwc">Classical Newari; Old Newari; Classical Nepal Bhasa</lang>
<lang code3="nya" code2="ny">Chichewa; Chewa; Nyanja</lang>
<lang code3="nym">Nyamwezi</lang>
<lang code3="nyn">Nyankole</lang>
<lang code3="nyo">Nyoro</lang>
<lang code3="nzi">Nzima</lang>
<lang code3="oci" code2="oc">Occitan (post 1500)</lang>
<lang code3="oji" code2="oj">Ojibwa</lang>
<lang code3="ori" code2="or">Oriya</lang>
<lang code3="orm" code2="om">Oromo</lang>
<lang code3="osa">Osage</lang>
<lang code3="oss" code2="os">Ossetian; Ossetic</lang>
<lang code3="ota">Turkish, Ottoman (1500-1928)</lang>
<lang code3="oto">Otomian languages</lang>
<lang code3="paa">Papuan languages</lang>
<lang code3="pag">Pangasinan</lang>
<lang code3="pal">Pahlavi</lang>
<lang code3="pam">Pampanga; Kapampangan</lang>
<lang code3="pan" code2="pa">Panjabi; Punjabi</lang>
<lang code3="pap">Papiamento</lang>
<lang code3="pau">Palauan</lang>
<lang code3="peo">Persian, Old (ca.600-400 B.C.)</lang>
<lang code3="per" code2="fa">Persian</lang>
<lang code3="phi">Philippine languages</lang>
<lang code3="phn">Phoenician</lang>
<lang code3="pli" code2="pi">Pali</lang>
<lang code3="pol" code2="pl">Polish</lang>
<lang code3="pon">Pohnpeian</lang>
<lang code3="por" code2="pt">Portuguese</lang>
<lang code3="pra">Prakrit languages</lang>
<lang code3="pro">Provençal, Old (to 1500); Occitan, Old (to 1500)</lang>
<lang code3="pus" code2="ps">Pushto; Pashto</lang>
<lang code3="qaa-qtz">Reserved for local use</lang>
<lang code3="que" code2="qu">Quechua</lang>
<lang code3="raj">Rajasthani</lang>
<lang code3="rap">Rapanui</lang>
<lang code3="rar">Rarotongan; Cook Islands Maori</lang>
<lang code3="roa">Romance languages</lang>
<lang code3="roh" code2="rm">Romansh</lang>
<lang code3="rom">Romany</lang>
<lang code3="rum" code2="ro">Romanian</lang>
<lang code3="run" code2="rn">Rundi</lang>
<lang code3="rup">Aromanian; Arumanian; Macedo-Romanian</lang>
<lang code3="rus" code2="ru">Russian</lang>
<lang code3="sad">Sandawe</lang>
<lang code3="sag" code2="sg">Sango</lang>
<lang code3="sah">Yakut</lang>
<lang code3="sai">South American Indian languages</lang>
<lang code3="sal">Salishan languages</lang>
<lang code3="sam">Samaritan Aramaic</lang>
<lang code3="san" code2="sa">Sanskrit</lang>
<lang code3="sas">Sasak</lang>
<lang code3="sat">Santali</lang>
<lang code3="scn">Sicilian</lang>
<lang code3="sco">Scots</lang>
<lang code3="sel">Selkup</lang>
<lang code3="sem">Semitic languages</lang>
<lang code3="sga">Irish, Old (to 900)</lang>
<lang code3="sgn">Sign Languages</lang>
<lang code3="shn">Shan</lang>
<lang code3="sid">Sidamo</lang>
<lang code3="sin" code2="si">Sinhala; Sinhalese</lang>
<lang code3="sio">Siouan languages</lang>
<lang code3="sit">Sino-Tibetan languages</lang>
<lang code3="sla">Slavic languages</lang>
<lang code3="slo" code2="sk">Slovak</lang>
<lang code3="slv" code2="sl">Slovenian</lang>
<lang code3="sma">Southern Sami</lang>
<lang code3="sme" code2="se">Northern Sami</lang>
<lang code3="smi">Sami languages</lang>
<lang code3="smj">Lule Sami</lang>
<lang code3="smn">Inari Sami</lang>
<lang code3="smo" code2="sm">Samoan</lang>
<lang code3="sms">Skolt Sami</lang>
<lang code3="sna" code2="sn">Shona</lang>
<lang code3="snd" code2="sd">Sindhi</lang>
<lang code3="snk">Soninke</lang>
<lang code3="sog">Sogdian</lang>
<lang code3="som" code2="so">Somali</lang>
<lang code3="son">Songhai languages</lang>
<lang code3="sot" code2="st">Sotho, Southern</lang>
<lang code3="spa" code2="es">Spanish; Castilian</lang>
<lang code3="srd" code2="sc">Sardinian</lang>
<lang code3="srn">Sranan Tongo</lang>
<lang code3="srp" code2="sr">Serbian</lang>
<lang code3="srr">Serer</lang>
<lang code3="ssa">Nilo-Saharan languages</lang>
<lang code3="ssw" code2="ss">Swati</lang>
<lang code3="suk">Sukuma</lang>
<lang code3="sun" code2="su">Sundanese</lang>
<lang code3="sus">Susu</lang>
<lang code3="sux">Sumerian</lang>
<lang code3="swa" code2="sw">Swahili</lang>
<lang code3="swe" code2="sv">Swedish</lang>
<lang code3="syc">Classical Syriac</lang>
<lang code3="syr">Syriac</lang>
<lang code3="tah" code2="ty">Tahitian</lang>
<lang code3="tai">Tai languages</lang>
<lang code3="tam" code2="ta">Tamil</lang>
<lang code3="tat" code2="tt">Tatar</lang>
<lang code3="tel" code2="te">Telugu</lang>
<lang code3="tem">Timne</lang>
<lang code3="ter">Tereno</lang>
<lang code3="tet">Tetum</lang>
<lang code3="tgk" code2="tg">Tajik</lang>
<lang code3="tgl" code2="tl">Tagalog</lang>
<lang code3="tha" code2="th">Thai</lang>
<lang code3="tib" code2="bo">Tibetan</lang>
<lang code3="tig">Tigre</lang>
<lang code3="tir" code2="ti">Tigrinya</lang>
<lang code3="tiv">Tiv</lang>
<lang code3="tkl">Tokelau</lang>
<lang code3="tlh">Klingon; tlhIngan-Hol</lang>
<lang code3="tli">Tlingit</lang>
<lang code3="tmh">Tamashek</lang>
<lang code3="tog">Tonga (Nyasa)</lang>
<lang code3="ton" code2="to">Tonga (Tonga Islands)</lang>
<lang code3="tpi">Tok Pisin</lang>
<lang code3="tsi">Tsimshian</lang>
<lang code3="tsn" code2="tn">Tswana</lang>
<lang code3="tso" code2="ts">Tsonga</lang>
<lang code3="tuk" code2="tk">Turkmen</lang>
<lang code3="tum">Tumbuka</lang>
<lang code3="tup">Tupi languages</lang>
<lang code3="tur" code2="tr">Turkish</lang>
<lang code3="tut">Altaic languages</lang>
<lang code3="tvl">Tuvalu</lang>
<lang code3="twi" code2="tw">Twi</lang>
<lang code3="tyv">Tuvinian</lang>
<lang code3="udm">Udmurt</lang>
<lang code3="uga">Ugaritic</lang>
<lang code3="uig" code2="ug">Uighur; Uyghur</lang>
<lang code3="ukr" code2="uk">Ukrainian</lang>
<lang code3="umb">Umbundu</lang>
<lang code3="und">Undetermined</lang>
<lang code3="urd" code2="ur">Urdu</lang>
<lang code3="uzb" code2="uz">Uzbek</lang>
<lang code3="vai">Vai</lang>
<lang code3="ven" code2="ve">Venda</lang>
<lang code3="vie" code2="vi">Vietnamese</lang>
<lang code3="vol" code2="vo">Volapük</lang>
<lang code3="vot">Votic</lang>
<lang code3="wak">Wakashan languages</lang>
<lang code3="wal">Wolaitta; Wolaytta</lang>
<lang code3="war">Waray</lang>
<lang code3="was">Washo</lang>
<lang code3="wel" code2="cy">Welsh</lang>
<lang code3="wen">Sorbian languages</lang>
<lang code3="wln" code2="wa">Walloon</lang>
<lang code3="wol" code2="wo">Wolof</lang>
<lang code3="xal">Kalmyk; Oirat</lang>
<lang code3="xho" code2="xh">Xhosa</lang>
<lang code3="yao">Yao</lang>
<lang code3="yap">Yapese</lang>
<lang code3="yid" code2="yi">Yiddish</lang>
<lang code3="yor" code2="yo">Yoruba</lang>
<lang code3="ypk">Yupik languages</lang>
<lang code3="zap">Zapotec</lang>
<lang code3="zbl">Blissymbols; Blissymbolics; Bliss</lang>
<lang code3="zen">Zenaga</lang>
<lang code3="zgh">Standard Moroccan Tamazight</lang>
<lang code3="zha" code2="za">Zhuang; Chuang</lang>
<lang code3="znd">Zande languages</lang>
<lang code3="zul" code2="zu">Zulu</lang>
<lang code3="zun">Zuni</lang>
<lang code3="zxx">No linguistic content; Not applicable</lang>
<lang code3="zza">Zaza; Dimili; Dimli; Kirdki; Kirmanjki; Zazaki</lang>
</xsl:variable>

<xsl:variable name="languageName">
<xsl:choose>
<xsl:when test="string-length($languageCode) = 3">
  <xsl:value-of select="msxsl:node-set($languageCodes)/lang[@code3=$languageCode]"/>
</xsl:when>
<xsl:when test="string-length($languageCode) = 2">
  <xsl:value-of select="msxsl:node-set($languageCodes)/lang[@code2=$languageCode]"/>
</xsl:when>
</xsl:choose>
</xsl:variable>

<xsl:choose>
<xsl:when test="$languageName != ''">
   <xsl:value-of select="$languageName"/>
</xsl:when>
<xsl:otherwise>
   <xsl:value-of select="$languageCode"/>
</xsl:otherwise>
</xsl:choose>
    </xsl:template>

    <xsl:template name="show-countryName">
      <xsl:param name="countryCode"/>
<xsl:variable name="countryCodes">
<country code="AF">Afghanistan</country>
<country code="AX">Åland Islands</country>
<country code="AL">Albania</country>
<country code="DZ">Algeria</country>
<country code="AS">American Samoa</country>
<country code="AD">Andorra</country>
<country code="AO">Angola</country>
<country code="AI">Anguilla</country>
<country code="AQ">Antarctica</country>
<country code="AG">Antigua And Barbuda</country>
<country code="AR">Argentina</country>
<country code="AM">Armenia</country>
<country code="AW">Aruba</country>
<country code="AU">Australia</country>
<country code="AT">Austria</country>
<country code="AZ">Azerbaijan</country>
<country code="BS">Bahamas</country>
<country code="BH">Bahrain</country>
<country code="BD">Bangladesh</country>
<country code="BB">Barbados</country>
<country code="BY">Belarus</country>
<country code="BE">Belgium</country>
<country code="BZ">Belize</country>
<country code="BJ">Benin</country>
<country code="BM">Bermuda</country>
<country code="BT">Bhutan</country>
<country code="BO">Bolivia</country>
<country code="BA">Bosnia And Herzegovina</country>
<country code="BW">Botswana</country>
<country code="BV">Bouvet Island</country>
<country code="BR">Brazil</country>
<country code="IO">British Indian Ocean Territory</country>
<country code="BN">Brunei Darussalam</country>
<country code="BG">Bulgaria</country>
<country code="BF">Burkina Faso</country>
<country code="BI">Burundi</country>
<country code="KH">Cambodia</country>
<country code="CM">Cameroon</country>
<country code="CA">Canada</country>
<country code="CV">Cape Verde</country>
<country code="KY">Cayman Islands</country>
<country code="CF">Central African Republic</country>
<country code="TD">Chad</country>
<country code="CL">Chile</country>
<country code="CN">China</country>
<country code="CX">Christmas Island</country>
<country code="CC">Cocos (Keeling) Islands</country>
<country code="CO">Colombia</country>
<country code="KM">Comoros</country>
<country code="CG">Congo</country>
<country code="CD">Congo, The Democratic Republic Of The</country>
<country code="CK">Cook Islands</country>
<country code="CR">Costa Rica</country>
<country code="CI">Côte D'Ivoire</country>
<country code="HR">Croatia</country>
<country code="CU">Cuba</country>
<country code="CY">Cyprus</country>
<country code="CZ">Czech Republic</country>
<country code="DK">Denmark</country>
<country code="DJ">Djibouti</country>
<country code="DM">Dominica</country>
<country code="DO">Dominican Republic</country>
<country code="EC">Ecuador</country>
<country code="EG">Egypt</country>
<country code="SV">El Salvador</country>
<country code="GQ">Equatorial Guinea</country>
<country code="ER">Eritrea</country>
<country code="EE">Estonia</country>
<country code="ET">Ethiopia</country>
<country code="FK">Falkland Islands (Malvinas)</country>
<country code="FO">Faroe Islands</country>
<country code="FJ">Fiji</country>
<country code="FI">Finland</country>
<country code="FR">France</country>
<country code="GF">French Guiana</country>
<country code="PF">French Polynesia</country>
<country code="TF">French Southern Territories</country>
<country code="GA">Gabon</country>
<country code="GM">Gambia</country>
<country code="GE">Georgia</country>
<country code="DE">Germany</country>
<country code="GH">Ghana</country>
<country code="GI">Gibraltar</country>
<country code="GR">Greece</country>
<country code="GL">Greenland</country>
<country code="GD">Grenada</country>
<country code="GP">Guadeloupe</country>
<country code="GU">Guam</country>
<country code="GT">Guatemala</country>
<country code="GG">Guernsey</country>
<country code="GN">Guinea</country>
<country code="GW">Guinea-Bissau</country>
<country code="GY">Guyana</country>
<country code="HT">Haiti</country>
<country code="HM">Heard Island And Mcdonald Islands</country>
<country code="VA">Holy See (Vatican City State)</country>
<country code="HN">Honduras</country>
<country code="HK">Hong Kong</country>
<country code="HU">Hungary</country>
<country code="IS">Iceland</country>
<country code="IN">India</country>
<country code="ID">Indonesia</country>
<country code="IR">Iran, Islamic Republic Of</country>
<country code="IQ">Iraq</country>
<country code="IE">Ireland</country>
<country code="IM">Isle Of Man</country>
<country code="IL">Israel</country>
<country code="IT">Italy</country>
<country code="JM">Jamaica</country>
<country code="JP">Japan</country>
<country code="JE">Jersey</country>
<country code="JO">Jordan</country>
<country code="KZ">Kazakhstan</country>
<country code="KE">Kenya</country>
<country code="KI">Kiribati</country>
<country code="KP">Korea, Democratic People's Republic Of</country>
<country code="KR">Korea, Republic Of</country>
<country code="KW">Kuwait</country>
<country code="KG">Kyrgyzstan</country>
<country code="LA">Lao People'S Democratic Republic</country>
<country code="LV">Latvia</country>
<country code="LB">Lebanon</country>
<country code="LS">Lesotho</country>
<country code="LR">Liberia</country>
<country code="LY">Libyan Arab Jamahiriya</country>
<country code="LI">Liechtenstein</country>
<country code="LT">Lithuania</country>
<country code="LU">Luxembourg</country>
<country code="MO">Macao</country>
<country code="MK">Macedonia, The Former Yugoslav Republic Of</country>
<country code="MG">Madagascar</country>
<country code="MW">Malawi</country>
<country code="MY">Malaysia</country>
<country code="MV">Maldives</country>
<country code="ML">Mali</country>
<country code="MT">Malta</country>
<country code="MH">Marshall Islands</country>
<country code="MQ">Martinique</country>
<country code="MR">Mauritania</country>
<country code="MU">Mauritius</country>
<country code="YT">Mayotte</country>
<country code="MX">Mexico</country>
<country code="FM">Micronesia, Federated States Of</country>
<country code="MD">Moldova, Republic Of</country>
<country code="MC">Monaco</country>
<country code="MN">Mongolia</country>
<country code="ME">Montenegro</country>
<country code="MS">Montserrat</country>
<country code="MA">Morocco</country>
<country code="MZ">Mozambique</country>
<country code="MM">Myanmar</country>
<country code="NA">Namibia</country>
<country code="NR">Nauru</country>
<country code="NP">Nepal</country>
<country code="NL">Netherlands</country>
<country code="AN">Netherlands Antilles</country>
<country code="NC">New Caledonia</country>
<country code="NZ">New Zealand</country>
<country code="NI">Nicaragua</country>
<country code="NE">Niger</country>
<country code="NG">Nigeria</country>
<country code="NU">Niue</country>
<country code="NF">Norfolk Island</country>
<country code="MP">Northern Mariana Islands</country>
<country code="NO">Norway</country>
<country code="OM">Oman</country>
<country code="PK">Pakistan</country>
<country code="PW">Palau</country>
<country code="PS">Palestinian Territory, Occupied</country>
<country code="PA">Panama</country>
<country code="PG">Papua New Guinea</country>
<country code="PY">Paraguay</country>
<country code="PE">Peru</country>
<country code="PH">Philippines</country>
<country code="PN">Pitcairn</country>
<country code="PL">Poland</country>
<country code="PT">Portugal</country>
<country code="PR">Puerto Rico</country>
<country code="QA">Qatar</country>
<country code="RE">Réunion</country>
<country code="RO">Romania</country>
<country code="RU">Russian Federation</country>
<country code="RW">Rwanda</country>
<country code="BL">Saint Barthélemy</country>
<country code="SH">Saint Helena</country>
<country code="KN">Saint Kitts And Nevis</country>
<country code="LC">Saint Lucia</country>
<country code="MF">Saint Martin</country>
<country code="PM">Saint Pierre And Miquelon</country>
<country code="VC">Saint Vincent And The Grenadines</country>
<country code="WS">Samoa</country>
<country code="SM">San Marino</country>
<country code="ST">Sao Tome And Principe</country>
<country code="SA">Saudi Arabia</country>
<country code="SN">Senegal</country>
<country code="RS">Serbia</country>
<country code="SC">Seychelles</country>
<country code="SL">Sierra Leone</country>
<country code="SG">Singapore</country>
<country code="SK">Slovakia</country>
<country code="SI">Slovenia</country>
<country code="SB">Solomon Islands</country>
<country code="SO">Somalia</country>
<country code="ZA">South Africa</country>
<country code="GS">South Georgia And The South Sandwich Islands</country>
<country code="ES">Spain</country>
<country code="LK">Sri Lanka</country>
<country code="SD">Sudan</country>
<country code="SR">Suriname</country>
<country code="SJ">Svalbard And Jan Mayen</country>
<country code="SZ">Swaziland</country>
<country code="SE">Sweden</country>
<country code="CH">Switzerland</country>
<country code="SY">Syrian Arab Republic</country>
<country code="TW">Taiwan, Province Of China</country>
<country code="TJ">Tajikistan</country>
<country code="TZ">Tanzania, United Republic Of</country>
<country code="TH">Thailand</country>
<country code="TL">Timor-Leste</country>
<country code="TG">Togo</country>
<country code="TK">Tokelau</country>
<country code="TO">Tonga</country>
<country code="TT">Trinidad And Tobago</country>
<country code="TN">Tunisia</country>
<country code="TR">Turkey</country>
<country code="TM">Turkmenistan</country>
<country code="TC">Turks And Caicos Islands</country>
<country code="TV">Tuvalu</country>
<country code="UG">Uganda</country>
<country code="UA">Ukraine</country>
<country code="AE">United Arab Emirates</country>
<country code="GB">United Kingdom</country>
<country code="US">United States</country>
<country code="UM">United States Minor Outlying Islands</country>
<country code="UY">Uruguay</country>
<country code="UZ">Uzbekistan</country>
<country code="VU">Vanuatu</country>
<country code="VE">Venezuela, Bolivarian Republic Of</country>
<country code="VN">Viet Nam</country>
<country code="VG">Virgin Islands, British</country>
<country code="VI">Virgin Islands, U.S.</country>
<country code="WF">Wallis And Futuna</country>
<country code="EH">Western Sahara</country>
<country code="YE">Yemen</country>
<country code="ZM">Zambia</country>
<country code="ZW">Zimbabwe</country>
</xsl:variable>

<xsl:variable name="countryName">
<xsl:value-of select="msxsl:node-set($countryCodes)/country[@code=$countryCode]"/>
</xsl:variable>

<xsl:choose>
<xsl:when test="$countryName != ''">
   <xsl:value-of select="$countryName"/>
</xsl:when>
<xsl:otherwise>
   <xsl:value-of select="$countryCode"/>
</xsl:otherwise>
</xsl:choose>
    </xsl:template>

  <xsl:template match="n1:languageCode" mode="ISO639LanguageISO3166Country">
    <xsl:variable name="languageName">
        <xsl:call-template name="show-languageName">
           <xsl:with-param name="languageCode">
            <xsl:choose>
                <xsl:when test="contains(@code, '-')">
                   <xsl:value-of select="substring-before(@code, '-')"/>
                </xsl:when>
                <xsl:otherwise>
                   <xsl:value-of select="@code"/>
                </xsl:otherwise>
            </xsl:choose>
           </xsl:with-param>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="countryName">
        <xsl:call-template name="show-countryName">
           <xsl:with-param name="countryCode" select="substring-after(@code, '-')"/>
        </xsl:call-template>
    </xsl:variable>

    <xsl:value-of select="$languageName"/>
    <xsl:if test="$countryName!=''">
      <xsl:text> (</xsl:text>
      <xsl:value-of select="$countryName"/>
      <xsl:text>)</xsl:text>
    </xsl:if>
  </xsl:template>
  
   <xsl:template name="addCSS">
      <style type="text/css">
         <xsl:text>
body {
  color: #003366;
  background-color: #FFFFFF;
  font-family: Verdana, Tahoma, sans-serif;
  font-size: 11px;
}

a {
  color: #003366;
  background-color: #FFFFFF;
}

h1 {
  font-size: 12pt;
  font-weight: bold;
}

h2 {
  font-size: 11pt;
  font-weight: bold;
}

h3 {
  font-size: 10pt;
  font-weight: bold;
}

h4 {
  font-size: 8pt;
  font-weight: bold;
}

div {
  width: 100%;
}

table {
  line-height: 10pt;
  width: 100%;
  border-collapse: collapse;
  empty-cells: show;
}

tr {
  background-color: #ccccff;
}

td {
  padding: 0.1cm 0.2cm;
  vertical-align: top;
}

.h1center {
  font-size: 12pt;
  font-weight: bold;
  text-align: center;
  width: 100%;
}

.header_table{
  border: 1pt inset #00008b;
}

.narr_table {
  width: 100%;
}

.narr_tr {
  background-color: #ffffcc;
}

.narr_th {
  background-color: #ffd700;
}

.td_label{
  font-weight: bold;
  color: white;
}
          </xsl:text>
      </style>
   </xsl:template>
</xsl:stylesheet>
