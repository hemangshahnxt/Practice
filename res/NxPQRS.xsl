<?xml version="1.0"?>

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:template match="/">
    <html>
      <head>
        <title>PQRS Report</title>
      </head>
      <body>
        File created on <xsl:value-of select="submission/file-audit-data/create-date"/> at <xsl:value-of select="submission/file-audit-data/create-time"/> by <xsl:value-of select="submission/file-audit-data/create-by"/>
		<h2>Reporting for providers:</h2>
        <xsl:for-each select="submission/measure-group/provider">
		  <h2>          
            NPI <xsl:value-of select="npi"/>
          </h2>
		</xsl:for-each>
        <table border="1">
        <tr bgcolor="#9acd32">
            <th>PQRS #</th>
            <th>NQF #</th>
            <th>CMS #</th>
            <th>Measure</th>
            <th>Eligible</th>
            <th>Meeting Performance</th>
            <th>Excluded From Performance</th>
            <th>Performance Not Met</th>
            <th>Reporting Rate</th>
            <th>Performance Rate</th>
        </tr>
		<xsl:for-each select="submission/measure-group/pqri-measure">
            <tr>
            <xsl:choose>
                <xsl:when test="pqri-measure-number='0002'">
                <td>N/A</td><td>0002</td><td>146</td><td>Appropriate Testing for Children with Pharyngitis</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0018'">
                <td>N/A</td><td>0018</td><td>165</td><td>Controlling High Blood Pressure</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0052'">
                <td>N/A</td><td>0052</td><td>166</td><td>Use of Imaging Studies for Low Back Pain</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0038'">
                <td>N/A</td><td>0038</td><td>117</td><td>Childhood Immunization Status</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='12'">
                <td>12</td><td>0086</td><td>143</td><td>Primary Open Angle Glaucoma (POAG): Optic Nerve Evaluation</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='113'">
                <td>113</td><td>0034</td><td>130</td><td>Colorectal Cancer Screening</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='3'">
                <td>3</td><td>0061</td><td>N/A</td><td>Diabetes: Blood Pressure Management</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0032'">
                <td>N/A</td><td>0032</td><td>124</td><td>Cervical Cancer Screening</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='111'">
                <td>111</td><td>0043</td><td>127</td><td>Preventive Care and Screening: Pneumonia Vaccination for Patients 65 Years and Older</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='110'">
                <td>110</td><td>0041</td><td>147</td><td>Preventive Care and Screening: Influenza Immunization </td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='128.1'">
                <td>128.1</td><td>0421-1</td><td>69</td><td>Preventive Care and Screening: Body Mass Index (BMI) Screening and Follow-Up - 1: Age &gt;= 65</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='128.2'">
                <td>128.2</td><td>0421-2</td><td>69</td><td>Preventive Care and Screening: Body Mass Index (BMI) Screening and Follow-Up - 2: Age &gt;= 18 and &lt; 65</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0013'">
                <td>N/A</td><td>0013</td><td>N/A</td><td>Hypertension: Blood Pressure Management</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0028'">
                <td>N/A</td><td>0028</td><td>138</td><td>Preventive Care and Screening: Tobacco Use: Screening and Cessation Intervention</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0028a'">
                <td>N/A</td><td>0028a</td><td>N/A</td><td>Preventive Care and Screening, pt. A - Tobacco Use Assessment</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0028b'">
                <td>N/A</td><td>0028b</td><td>N/A</td><td>Preventive Care and Screening, pt. B - Tobacco Cessation Intervention</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='204'">
                <td>204</td><td>0068</td><td>164</td><td>Ischemic Vascular Disease (IVD): Use of aspirin or Another Antithrombotic</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='72'">
                <td>72</td><td>0385</td><td>141</td><td>Colon Cancer: Chemotherapy for AJCC Stage III Colon Cancer Patients</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='1'">
                <td>1</td><td>0059</td><td>122</td><td>Diabetes Mellitus: Hemoglobin A1c Poor Control in Diabetes Mellitus</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='8'">
                <td>8</td><td>0083</td><td>144</td><td>Heart Failure: Beta-Blocker Therapy for Left Ventricular Systolic Dysfunction (LVSD)</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='102'">
                <td>102</td><td>0389</td><td>129</td><td>Prostate Cancer: Avoidance of Overuse of Bone Scan for Staging Low-Risk Prostate Cancer Patients</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='115.1'">
                <td>115.1</td><td>0027a</td><td>N/A</td><td>Preventive Care and Screening: Advising Smokers and Tobacco Users to Quit - A</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='115.2'">
                <td>115.2</td><td>0027b</td><td>N/A</td><td>Preventive Care and Screening: Advising Smokers and Tobacco Users to Quit - B</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='18'">
                <td>18</td><td>0088</td><td>167</td><td>Diabetic Retinopathy: Documentation of Presence or Absence of Macular Edema and Level of Severity of Retinopathy</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='6'">
                <td>6</td><td>0067</td><td>N/A</td><td>Coronary Artery Disease (CAD): Oral Antiplatelet Therapy Prescribed for Patients with CAD</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='5'">
                <td>5</td><td>0081</td><td>135</td><td>Heart Failure (HF): Angiotensin-Converting Enzyme (ACE) Inhibitor or Angiotensin Receptor Blocker (ARB) Therapy for Left Ventricular Systolic Dysfunction (LVSD)</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='64'">
                <td>64</td><td>0001</td><td>N/A</td><td>Asthma Assessment</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='7'">
                <td>7</td><td>0070</td><td>N/A</td><td>Coronary Artery Disease (CAD): Beta-Blocker Therapy for CAD Patients with Prior Myocardial Infarction (MI)</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='7.1'">
                <td>7.1</td><td>0070-1</td><td>145</td><td>Coronary Artery Disease (CAD): Left Ventricular Systolic Dysfunction (LVEF &lt;40%) - 1</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='7.2'">
                <td>7.2</td><td>0070-2</td><td>145</td><td>Coronary Artery Disease (CAD): Beta-Blocker Therapy-Prior Myocardial Infarction (MI) - 2</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='71'">
                <td>71</td><td>0387</td><td>N/A</td><td>Breast Cancer: Hormonal Therapy for Stage IC-IIIC Estrogen Receptor/Progesterone Receptor (ER/PR) Positive Breast Cancer</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='19'">
                <td>19</td><td>0089</td><td>142</td><td>Diabetic Retinopathy: Communication with the Physician Managing Ongoing Diabetes Care</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='197'">
                <td>197</td><td>0074</td><td>N/A</td><td>Coronary Artery Disease (CAD): Drug Therapy for Lowering LDL-Cholesterol</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='201'">
                <td>201</td><td>0073</td><td>N/A</td><td>Ischemic Vascular Disease (IVD): Blood Pressure Management</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='2'">
                <td>2</td><td>0064</td><td>163</td><td>Diabetes: Low Density Lipoprotein (LDL) Management</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='119'">
                <td>119</td><td>0062</td><td>134</td><td>Diabetes: Urine Protein Screening</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='112'">
                <td>112</td><td>0031</td><td>125</td><td>Breast Cancer Screening</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='163'">
                <td>163</td><td>0056</td><td>123</td><td>Diabetes: Foot Exam</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='117'">
                <td>117</td><td>0055</td><td>131</td><td>Diabetes: Eye Exam</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='200'">
                <td>200</td><td>0084</td><td>N/A</td><td>Heart Failure: Warfarin Therapy for Patients with Atrial Fibrillation</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0024'">
                <td>N/A</td><td>0024</td><td>N/A</td><td>Weight Assessment and Counseling for Children and Adolescents</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0024-1'">
                <td>N/A</td><td>0024-1</td><td>155</td><td>Weight Assessment and Counseling for Nutrition and Physical Activity for Children and Adolescents - 1: BMI Documented</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0024-1a'">
                <td>N/A</td><td>0024-1</td><td>155</td><td>Weight Assessment and Counseling for Nutrition and Physical Activity for Children and Adolescents - 1: BMI Documented - a: Ages 3-11</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0024-1b'">
                <td>N/A</td><td>0024-1</td><td>155</td><td>Weight Assessment and Counseling for Nutrition and Physical Activity for Children and Adolescents - 1: BMI Documented - b: Ages 12-17</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0024-2'">
                <td>N/A</td><td>0024-2</td><td>155</td><td>Weight Assessment and Counseling for Nutrition and Physical Activity for Children and Adolescents - 2: Nutrition Counseling</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0024-2a'">
                <td>N/A</td><td>0024-2</td><td>155</td><td>Weight Assessment and Counseling for Nutrition and Physical Activity for Children and Adolescents - 2: Nutrition Counseling - a: Ages 3-11</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0024-2b'">
                <td>N/A</td><td>0024-2</td><td>155</td><td>Weight Assessment and Counseling for Nutrition and Physical Activity for Children and Adolescents - 2: Nutrition Counseling - b: Ages 12-17</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0024-3'">
                <td>N/A</td><td>0024-3</td><td>155</td><td>Weight Assessment and Counseling for Nutrition and Physical Activity for Children and Adolescents - 3: Physical Activity Counseling</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0024-3a'">
                <td>N/A</td><td>0024-3</td><td>155</td><td>Weight Assessment and Counseling for Nutrition and Physical Activity for Children and Adolescents - 3: Physical Activity Counseling - a: Ages 3-11</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0024-3b'">
                <td>N/A</td><td>0024-3</td><td>155</td><td>Weight Assessment and Counseling for Nutrition and Physical Activity for Children and Adolescents - 3: Physical Activity Counseling - b: Ages 12-17</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='50'">
                <td>N/A</td><td>N/A</td><td>50</td><td>Closing the referral loop: receipt of specialist report</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='65'">
                <td>N/A</td><td>N/A</td><td>65</td><td>Hypertension: Improvement in Blood Pressure</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0022-1'">
                <td>N/A</td><td>0022-1</td><td>156</td><td>Use of High-Risk Medications in the Elderly - 1: At least one high-risk medication</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0022-2'">
                <td>N/A</td><td>0022-2</td><td>156</td><td>Use of High-Risk Medications in the Elderly - 2: At least two high-risk medications</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0419'">
                <td>N/A</td><td>0419</td><td>68</td><td>Documentation of Current Medications in the Medical Record</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='61'">
                <td>N/A</td><td>N/A</td><td>61</td><td>Preventive Care and Screening: Cholesterol - Fasting Low Density Lipoprotein (LDL-C) Test Performed</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='22'">
                <td>N/A</td><td>N/A</td><td>22</td><td>Preventive Care and Screening: Screening for High Blood Pressure and Follow-Up Documented</td>
                </xsl:when>
                <xsl:when test="pqri-measure-number='0564'">
                <td>N/A</td><td>0564</td><td>132</td><td>Cataracts: Complications within 30 Days Following Cataract Surgery Requiring Additional Surgical Procedures</td>
                </xsl:when>
                <xsl:otherwise>
                    <td></td>
                    <td></td>
                    <td>
                    <xsl:value-of select="pqri-measure-number"/>
                    </td>
                </xsl:otherwise>
                </xsl:choose>                  
            <td>
                <xsl:value-of select="eligible-instances"/>
            </td>
            <td>
                <xsl:value-of select="meets-performance-instances"/>
            </td>
            <td>
                <xsl:value-of select="performance-exclusion-instances"/>
            </td>
            <td>
                <xsl:value-of select="performance-not-met-instances"/>
            </td>
            <td>
                <xsl:value-of select="reporting-rate"/>
            </td>
            <td>
                <xsl:choose>
                <xsl:when test="performance-rate=''">
                    N/A
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="performance-rate"/>
                </xsl:otherwise>
                </xsl:choose>                  
            </td>
            </tr>
		</xsl:for-each>
        </table>
      </body>
    </html>
  </xsl:template>

</xsl:stylesheet>