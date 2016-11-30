// ImplementationDlg.cpp : implementation file
//
// (j.gruber 2008-12-22 09:42) - PLID 28023 - tab created
#include "stdafx.h"
#include "patientsRc.h"
#include "ImplementationDlg.h"
#include "globalutils.h"
#include "globaldatautils.h"
#include "ConfigureImplementationLadderDlg.h"
#include "ConfigureImplementationStepDlg.h"
#include "ImplementationLadderPickerDlg.h"
#include "mergeengine.h"
#include "Letterwriting.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"
#include "ImplementationStepCriteriaSelectionDlg.h"
#include "editComboBox.h"
#include "ImplementationEditSpecialistDlg.h"
#include "TodoUtils.h"

#include "AuditTrail.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_INSERT_ABOVE 54965
#define ID_INSERT_BELOW 54966
#define ID_DELETE_STEP 54967

//color definition
#define EMR_SPEC_NOT_ACTIVE RGB(229,59,59) //red
#define EMR_SPEC_DONE RGB(79,209,76) //green
#define EMR_SPEC_IN_PROGRESS RGB(253,251,122)  //yellow


// (j.gruber 2007-11-08 16:15) - PLID 28026 - added specialist and status datalists
enum SpecialistColumns {
	spcID = 0,
	spcName,
};


enum StatusColumn {
	stcID = 0,
	stcName,
};


//this is also used in ImplementationStepCriteriaSelectionDlg.cpp
enum ListType {
	lttProcedures = 1,
	lttEMR,
};


enum LadderColumns {
	lcLadderID = 0,
    lcLadderTypeID,
	lcStepID,
	lcSortOrder,
	lcActive,
	lcActiveDate,
	lcLadderName,
	lcNotes,
	lcActionID,
	lcAction,
	lcDocCategoryID,
	lcDocAssigned,
	lcDocDone,
	lcDone,

};

// (j.gruber 2008-05-22 11:27) - PLID 30144 - added status type list
enum TypeList {
	tlcID = 0,
	tlcName,
};

enum LadderTypes {
	ltEMR = 1,
	ltPM,
	ltSales,
};




//this is copied in configureimplementationstepdlg.cpp
//and is in data
//IT CANNOT BE CHANGED!!!
enum TimeInterval {
	tiImmediately = 0,
	tiDays,
	tiWeeks,
	tiMonths,
	tiYears,
};



//this is copied in configureimplementationstepdlg.cpp
//and is in data
//IT CANNOT BE CHANGED!!!
// (j.gruber 2008-05-22 12:35) - PLID 29445 - adding a custom step
enum ActionTypes {
	atEmailDocument = 1,
	atCompleteEMR,
	atSelectProcedures,
	atCustomizeTemplates,
	atMergeDocument,
	atMergePacket,
	atAptByCategory,
	atAptByType,
	atCustom,
};

/* Queries used to create these fields

For Creation of tables:

  CREATE TABLE EMRSpecialistT (UserID INT PRIMARY KEY NOT NULL CONSTRAINT EMRSpecialistT_UserID_UsersT_ID REFERENCES UsersT(PersonID))

  CREATE TABLE EMRStatusT(ID INT PRIMARY KEY, StatusName nVarChar(255) NOT NULL default '')

  // (j.gruber 2008-05-22 11:30) - PLID 30144 - EMRTypeT
  CREATE TABLE EMRTypeT(ID INT PRIMARY KEY, TypeName nVarChar(255) NOT NULL default '')


For NxClientsT Fields

  ALTER TABLE NxClientsT ADD EMRSpecialistID INT NULL CONSTRAINT NxClientsT_EMRSpecialistID_EMRSpecialistT_UserID REFERENCES EMRSpecialistT(UserID)

  ALTER TABLE NxClientsT ADD EMRStatusID INT NULL CONSTRAINT NxClientsT_EMRStatusID_EMRStatusT_ID REFERENCES EMRStatusT(ID)

  // (j.gruber 2008-05-22 11:30) - PLID 30144 - EMRTypeT
  ALTER TABLE NxClientsT ADD EMRTypeID INT NULL CONSTRAINT NxClientsT_EMRTypeID_EMRTypeT_ID REFERENCES EMRTypeT(ID)

  // (j.gruber 2008-05-22 14:43) - PLID 29437 - added followup date
  ALTER TABLE NxClientsT ADD EMRFollowupDate DateTime NULL 


To Insert Users Into EMRSpecialistT

  INSERT INTO EMRSpecialistT (UserID) VALUES (<UserID you want to add>)

To Insert Statii into EMRSpecialistT
  INSERT INTO EMRStatusT (ID, StatusName) VALUES (<nextID>, <name>)


  Alter Table NxClientsT ADD EMRPointPerson nVarChar(1000), EMRPointPersonEmail nVarChar(1000), 
	     PMPointPerson nVarChar(1000), PMPointPersonEmail nVarChar(1000)

  Alter Table NxClientsT ADD EMRStatusNote nVarChar(1000), EMRStatusDate datetime

Client Implementation Ladders, etc

  
  CREATE TABLE ClientImplementationLadderT (ID INT PRIMARY KEY, 
  ClientID INT CONSTRAINT CliImpLadderT_ClientID_PatientsT_PersonID REFERENCES PatientsT(PersonID),
  MasterLadderTemplateID INT CONSTRAINT CliImpLadderT_LadderTemplateID_ImpLadderTempT_ID References ImplementationLadderTemplateT(ID),
  Name nVarChar(255), Active bit not null default 1, 
  TypeID INT NULL CONSTRAINT CliImpLadderT_TypeID_ImpLadderTempTypeT_ID REFERENCES ImplementationLadderTemplateTypeT(ID),
  Done bit NOT NULL default 0)

  CREATE TABLE ClientImplementationStepsT (ID INT PRIMARY KEY, 
  LadderID INT CONSTRAINT CliImpStepT_LadderID_CliImpLadderT_ID REFERENCES ClientImplementationLadderT(ID), 
  Active bit not null default 0,
  ActionID INT NOT NULL, SortOrder INT NOT NULL,
  StepName NVarChar(255), Note nVarChar(1000), CreateToDo bit not null default 0, 
  ToDoUserID INT NULL,
  ToDoPriority INT NULL, ToDoCategoryID INT CONSTRAINT CliImpStepT_ToDoCategoryID_NoteCatsF_ID REFERENCES NoteCatsF(ID),
  IsClientAssigned bit not null default 0, IsWebVisible bit not null default 0,
  ClientDone bit not null default 0,
  Done bit not null default 0,
  DocumentCategoryID INT CONSTRAINT CliImpStepT_DocumentCategoryID_NoteCatsF_ID REFERENCES NoteCatsF(ID),
  ToDoTaskID INT NULL CONSTRAINT CliImpStepT_ToDoTaskID_ToDoList_TaskID REFERENCES ToDoList(TaskID),
  ActiveDate datetime NULL,
  AutoActivate bit NOT NULL default 0,
  ActivateTimeLength INT NULL,
  ActivateTimeInterval INT NULL)

  CREATE TABLE ClientImplementationStepCriteriaT (StepID INT CONSTRAINT   CliImpStepCriteriaT_StepID_CliImpStepT_ID REFERENCES ClientImplementationStepsT(ID), 
  ActionItemID INT, ActionItemPath nVarChar(1000),
   ClientVisible bit not null default 0,
  ClientComplete bit not null default 0,
  Complete bit not null default 0)


ImplementationProcedureT

  CREATE TABLE ImplementationProcedureT(ID INT PRIMARY KEY NOT NULL, Name nVarChar(500))
  
	//statments to insert records into table (Came from ProcedureT in Official Content)
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (3, 'Abdominoplasty')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (462, 'Abdominoplasty - Extended')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (334, 'Abdominoplasty - Mini')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (461, 'Abdominoplasty - Standard')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (458, 'Acne Treatment')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (560, 'Acupuncture')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (296, 'Aquamid')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (282, 'Artecoll')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (293, 'Autologen')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (437, 'BClear')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (309, 'Beta Peel')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (297, 'Bio-Alcamid')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (436, 'Biopsy')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (304, 'Bi-Phasic Peel')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (15, 'Blepharoplasty')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (335, 'Blepharoplasty Lower - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (467, 'Blepharoplasty Lower - Left Side')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (468, 'Blepharoplasty Lower - Right Side')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (336, 'Blepharoplasty Upper - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (469, 'Blepharoplasty Upper - Left Side')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (470, 'Blepharoplasty Upper - Right Side')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (162, 'Blue Peel')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (316, 'Body Lift')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (558, 'Body Therapy')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (338, 'Botox ®')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (232, 'Brachioplasty')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (473, 'Brachioplasty - Mini')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (475, 'Brachioplasty - Modified')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (477, 'Brachioplasty - Revision')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (476, 'Brachioplasty - Standard')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (4, 'Breast Augmentation')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (339, 'Breast Augmentation Subglandular - Bilateral - Saline Implants')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (478, 'Breast Augmentation Subglandular - Bilateral - Silicone Implants')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (340, 'Breast Augmentation Subglandular - Left Breast - Saline Implant')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (479, 'Breast Augmentation Subglandular - Left Breast - Silicone Implant')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (341, 'Breast Augmentation Subglandular - Right Breast - Saline Implant')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (480, 'Breast Augmentation Subglandular - Right Breast - Silicone Implant')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (342, 'Breast Augmentation Submuscular - Bilateral - Saline Implants')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (459, 'Breast Augmentation Submuscular - Bilateral - Silicone Implants')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (343, 'Breast Augmentation Submuscular - Left Breast - Saline Implant')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (481, 'Breast Augmentation Submuscular - Left Breast - Silicone Implant')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (344, 'Breast Augmentation Submuscular - Right Breast - Saline Implant')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (482, 'Breast Augmentation Submuscular - Right Breast - Silicone Implant')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (451, 'Breast Implant Exchange')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (545, 'Breast Implant Removal')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (382, 'Breast Implant Removal - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (483, 'Breast Implant Removal - Left Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (484, 'Breast Implant Removal - Right Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (222, 'Breast Reconstruction')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (346, 'Breast Reconstruction - Latissimus Muscle Flap - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (347, 'Breast Reconstruction - Latissimus Muscle Flap - Left Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (348, 'Breast Reconstruction - Latissimus Muscle Flap - Right Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (349, 'Breast Reconstruction Saline Implant - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (350, 'Breast Reconstruction Saline Implant - Left Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (351, 'Breast Reconstruction Saline Implant - Right Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (499, 'Breast Reconstruction Silicone Implant - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (500, 'Breast Reconstruction Silicone Implant - Left Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (501, 'Breast Reconstruction Silicone Implant - Right Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (352, 'Breast Reconstruction Tissue Expander - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (353, 'Breast Reconstruction Tissue Expander - Left Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (354, 'Breast Reconstruction Tissue Expander - Right Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (355, 'Breast Reconstruction TRAM Flap - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (356, 'Breast Reconstruction TRAM Flap - Left Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (357, 'Breast Reconstruction TRAM Flap- Right Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (18, 'Breast Reduction')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (546, 'Breast Reduction - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (358, 'Breast Reduction - Left Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (359, 'Breast Reduction - Right Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (506, 'Breast Reduction Free Nipple Graft - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (507, 'Breast Reduction Free Nipple Graft - Left Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (508, 'Breast Reduction Free Nipple Graft - Right Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (509, 'Breast Reduction Liposuction - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (510, 'Breast Reduction Liposuction - Left Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (511, 'Breast Reduction Liposuction - Right Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (238, 'Breast Reduction Reconstructive')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (62, 'Browlift')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (360, 'Browlift - Endoscopic')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (361, 'Browlift - Open')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (196, 'Burns Upper Extremity')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (362, 'Buttock Lift')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (472, 'Canthopexy')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (444, 'Capsuloplasty')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (287, 'Captique')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (363, 'Carpal Tunnel Release')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (520, 'Carpal Tunnel Release - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (216, 'Carpal Tunnel Release - Left Hand')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (315, 'Carpal Tunnel Release - Right Hand')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (69, 'Cheek Implant')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (442, 'Cheek Lift')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (161, 'Chemical Peel/Skin Treatments')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (67, 'Chin Augmentation')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (206, 'Cleft Lip/Palate')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (186, 'Collagen')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (277, 'Cosmoderm')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (276, 'Cosmoplast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (439, 'Craniosynostosis')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (397, 'Cryotherapy')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (291, 'Cymetra')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (235, 'Dacryocystorhinostomy')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (66, 'Dermabrasion')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (399, 'Dermal Filler')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (295, 'Dermalive')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (549, 'Dermalogen')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (236, 'Dupuytren''s Contracture Release')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (441, 'Ear Piercing')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (81, 'Earlobe Repair')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (318, 'Endermologie')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (234, 'Excision Hand/Wrist Ganglion')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (466, 'Excision of Lesion with Delayed Reconstruction')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (465, 'Excision of Lesion with Immediate Reconstruction')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (522, 'Excision Skin Lesion with Complex Closure')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (521, 'Excision Skin Lesion with Layered Closure')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (523, 'Excision Skin Lesion with Simple Closure')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (524, 'Excision Skin Tags')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (364, 'Extensor Tendon Repair')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (365, 'Facelift')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (512, 'Facelift - Full')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (372, 'Facelift - Lower')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (377, 'Facelift - Mid')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (264, 'Facial Reconstruction')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (402, 'Facials')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (292, 'Fascian')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (187, 'Fat Injection')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (366, 'Flexor Tendon Repair')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (367, 'Ganglion Cyst')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (303, 'Glycolic Peel')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (125, 'Gynecomastia')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (517, 'Gynecomastia - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (518, 'Gynecomastia - Left Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (519, 'Gynecomastia - Right Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (70, 'Hair Transplant')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (332, 'Hand Duplication Surgery')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (326, 'Hand Nerve/ Artery Surgery')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (128, 'Hand Surgery')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (275, 'Hylaform')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (408, 'Injectable Wrinkle Treatment')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (305, 'Jessners Peel')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (281, 'Juvederm')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (179, 'Laceration Repair')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (306, 'Lactic/Phenol Peel')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (203, 'Lap-Band Gastric Restriction')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (181, 'Laser Hair Removal')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (557, 'Laser Massage')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (14, 'Laser Skin Resurfacing')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (411, 'Laser Tattoo Removal')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (272, 'Laser Treatment of Pigmented Lesion(s)')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (83, 'Laser Treatment of Vascular Lesion')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (312, 'Laser Treatment of Wart(s)')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (556, 'Lasik Treatment')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (124, 'Leg Vein Treatment')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (554, 'Light Therapy')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (258, 'Lip Augmentation')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (40, 'Liposuction')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (525, 'Liposuction - Abdomen')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (219, 'Liposuction - Arms')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (526, 'Liposuction - Axillae/Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (527, 'Liposuction - Back')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (369, 'Liposuction - Face')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (528, 'Liposuction - Flanks')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (529, 'Liposuction - Hips')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (370, 'Liposuction - Legs')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (530, 'Liposuction - Neck')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (531, 'Liposuction - Revision')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (209, 'Liposuction - Submental')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (371, 'Liposuction - Trunk')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (331, 'Lower Body Lift')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (551, 'Lymphatic Drainage Massage')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (17, 'Mastopexy')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (373, 'Mastopexy - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (374, 'Mastopexy - Left Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (375, 'Mastopexy - Right Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (298, 'Matridex')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (288, 'Matridur')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (548, 'Mesotherapy')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (221, 'Microdermabrasion')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (190, 'Micropeel')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (112, 'Mohs Micrographic Surgery')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (422, 'Mole Removal')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (327, 'Nail Surgery')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (378, 'Nasal Injury Repair')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (68, 'Necklift')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (379, 'Nipple Reconstruction - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (485, 'Nipple Reconstruction - Left Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (486, 'Nipple Reconstruction - Right Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (368, 'Open Capsulectomy - Saline Implant Exchange - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (491, 'Open Capsulectomy - Saline Implant Exchange - Left Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (492, 'Open Capsulectomy - Saline Implant Exchange - Right Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (443, 'Open Capsulectomy - Silicone Implant Exchange - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (494, 'Open Capsulectomy - Silicone Implant Exchange - Left Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (493, 'Open Capsulectomy - Silicone Implant Exchange - Right Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (460, 'Open Capsulotomy - Saline Implant Exchange - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (489, 'Open Capsulotomy - Saline Implant Exchange - Left Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (490, 'Open Capsulotomy - Saline Implant Exchange - Right Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (445, 'Open Capsulotomy - Silicone Implant Exchange - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (487, 'Open Capsulotomy - Silicone Implant Exchange - Left Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (488, 'Open Capsulotomy - Silicone Implant Exchange - Right Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (16, 'Otoplasty')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (533, 'Otoplasty - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (380, 'Otoplasty - Left Ear')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (381, 'Otoplasty - Right Ear')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (286, 'Perlane')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (425, 'Phenol Chemical Peel')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (319, 'Phlebectomy')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (285, 'Photodynamic Therapy')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (426, 'Photofacial')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (214, 'Phototherapy')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (294, 'Plasmagel')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (438, 'Positional Molding')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (555, 'Radiance')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (265, 'Radiesse')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (553, 'Reflexology')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (290, 'Resoplast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (266, 'Restylane ®')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (2, 'Rhinoplasty')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (534, 'Rhinoplasty - Dorsum')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (383, 'Rhinoplasty - Tip')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (307, 'Salicylic Peel')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (213, 'Scar Revision')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (539, 'Scar Revision - Inframammary fold - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (537, 'Scar Revision - Inframammary fold - Left Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (538, 'Scar Revision - Inframammary fold - Right Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (535, 'Scar Revision - Lower Abdomen')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (542, 'Scar Revision - Periareolar - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (540, 'Scar Revision - Periareolar - Left Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (541, 'Scar Revision - Periareolar - Right Breast')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (536, 'Scar Revision - Umbillicus')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (431, 'Sclerotherapy')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (280, 'Sculptra')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (180, 'Septoplasty')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (283, 'Silicone')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (384, 'Skin Cancer')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (329, 'Skin Graft')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (385, 'Skin Lesion/Tumor')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (432, 'TCA Chemical Peel')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (308, 'TCA Combo Peel')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (386, 'Tenolysis')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (314, 'Thermage')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (376, 'Thigh Lift')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (71, 'Thigh Lift - Medial - Bilateral')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (387, 'Trigger Finger')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (561, 'Waxing')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (278, 'Zyderm I')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (279, 'Zyderm II')
	INSERT INTO ImplementationProcedureT (ID, Name) VALUES (289, 'Zyplast')

  
	CREATE TABLE ImplementationEMRTemplatesT(ID INT PRIMARY KEY NOT NULL, Name nVarChar(500))
	  
	
	*******NOT ADDED TO INTERNAL YET!!!***********************
	CREATE TABLE ImplementationProcEMRConnectT(ID INT NOT NULL, 
	ImpProcID INT NOT NULL CONSTRAINT ImpProcEMRConnT_ImpProc_ID_ImpProcT_ID REFERENCES ImplementationProcedureT(ID),
	ImpEMRTemplateID INT NOT NULL CONSTRAINT ImpProcEMRConnT_ImpEMRTempID_ImpEMRTempT_ID REFERENCES ImplementationEMRTemplatesT(ID))


	ALTER TABLE NxClientsT ADD SecondaryEMRSpecialistID INT NULL CONSTRAINT NxClientsT_SecEMRSpecialistID_EMRSpecialistT_UserID REFERENCES EMRSpecialistT(UserID)
  

  

*/

/////////////////////////////////////////////////////////////////////////////
// CImplementationDlg dialog


CImplementationDlg::CImplementationDlg(CWnd* pParent)
	: CPatientDialog(CImplementationDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImplementationDlg)
	//}}AFX_DATA_INIT

	// (r.gonet 06/21/2010) - PLID 38871 - Initialize the old values for auditing purposes
	m_strOldEmrSpecialistID = "-1";
	m_strOldEmrSpecialist = "";
	m_strOldEmrStatusID = "-1";
	m_strOldEmrStatus = "";
	m_strOldEmrTypeID = "-1";
	m_strOldEmrType = "";
	m_strOldStatusNote = "";
	m_strOldEmrClientPointPerson = "";
	m_strOldEmrPointPersonEmail = "";
	m_strOldPMClientPointPerson = "";
	m_strOldPMPointPersonEmail = "";
	// (j.gruber 2010-07-29 14:08) - PLID 39878
	m_strOldSecEmrSpecID = "-1";
	m_strOldSecEmrSpec = "";
}


void CImplementationDlg::DoDataExchange(CDataExchange* pDX)
{
	CPatientDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImplementationDlg)
	DDX_Control(pDX, IDC_EDIT_EMR_SPECIALIST_LIST, m_btnEditSpecialist);
	DDX_Control(pDX, IDC_EDIT_EMR_STATUS_LIST, m_btnEditStatus);
	DDX_Control(pDX, IDC_CLIENT_IMAGE, m_btnImage);
	DDX_Control(pDX, IDC_LADDER_SETUP, m_btn_LadderSetup);
	DDX_Control(pDX, IDC_MOVE_STEP_DOWN, m_btn_Down);
	DDX_Control(pDX, IDC_MOVE_STEP_UP, m_btn_Up);
	DDX_Control(pDX, IDC_DELETE_IMPLEMENTATION_LADDER, m_btn_DeleteLadder);
	DDX_Control(pDX, IDC_ADD_IMPLEMENTATION_LADDER, m_btn_AddLadder);
	DDX_Control(pDX, IDC_AUTO_COMPLETE, m_btn_AutoComplete);
	DDX_Control(pDX, IDC_STATUS_NOTE, m_nxeditStatusNote);
	DDX_Control(pDX, IDC_CLIENT_WEBSITE, m_nxeditClientWebsite);
	DDX_Control(pDX, IDC_EMR_POINT_PERSON, m_nxeditEmrPointPerson);
	DDX_Control(pDX, IDC_EMR_POINT_EMAIL, m_nxeditEmrPointEmail);
	DDX_Control(pDX, IDC_PM_POINT_PERSON, m_nxeditPmPointPerson);
	DDX_Control(pDX, IDC_PM_POINT_EMAIL, m_nxeditPmPointEmail);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CImplementationDlg, CPatientDialog)
	//{{AFX_MSG_MAP(CImplementationDlg)
	ON_BN_CLICKED(IDC_LADDER_SETUP, OnLadderSetup)
	ON_BN_CLICKED(IDC_ADD_IMPLEMENTATION_LADDER, OnAddImplementationLadder)
	ON_EN_KILLFOCUS(IDC_EMR_POINT_PERSON, OnKillfocusEmrPointPerson)
	ON_EN_KILLFOCUS(IDC_EMR_POINT_EMAIL, OnKillfocusEmrPointEmail)
	ON_EN_KILLFOCUS(IDC_PM_POINT_EMAIL, OnKillfocusPmPointEmail)
	ON_EN_KILLFOCUS(IDC_PM_POINT_PERSON, OnKillfocusPmPointPerson)
	ON_BN_CLICKED(IDC_MOVE_STEP_UP, OnMoveStepUp)
	ON_BN_CLICKED(IDC_MOVE_STEP_DOWN, OnMoveStepDown)
	ON_EN_KILLFOCUS(IDC_STATUS_NOTE, OnKillfocusStatusNote)
	ON_BN_CLICKED(IDC_DELETE_IMPLEMENTATION_LADDER, OnDeleteImplementationLadder)
	ON_BN_CLICKED(IDC_AUTO_COMPLETE, OnAutoComplete)
	ON_BN_CLICKED(IDC_GO_WEBSITE, OnGoWebsite)
	ON_BN_CLICKED(IDC_EDIT_EMR_SPECIALIST_LIST, OnEditSpecialist)
	ON_BN_CLICKED(IDC_EDIT_EMR_STATUS_LIST, OnEditStatus)
	ON_BN_CLICKED(IDC_EDIT_EMR_TYPE_LIST, OnEditEmrTypeList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImplementationDlg message handlers

BOOL CImplementationDlg::OnInitDialog() 
{
	try {
		CPatientDialog::OnInitDialog();

		m_btn_Up.AutoSet(NXB_UP);
		m_btn_Down.AutoSet(NXB_DOWN);
		m_btn_LadderSetup.AutoSet(NXB_NEW);
		m_btn_AddLadder.AutoSet(NXB_NEW);
		m_btn_DeleteLadder.AutoSet(NXB_DELETE);
		m_btn_AutoComplete.AutoSet(NXB_MODIFY);

		m_pSpecialistList = BindNxDataList2Ctrl(IDC_EMR_SPECIALIST_LIST, TRUE);
		m_pSecSpecList = BindNxDataList2Ctrl(IDC_SEC_EMR_SPECIALIST_LIST, TRUE);
		m_pStatusList = BindNxDataList2Ctrl(IDC_EMR_STATUS_LIST, TRUE);
		m_pTypeList = BindNxDataList2Ctrl(IDC_EMR_TYPE_LIST, TRUE);
		m_pLadderList = BindNxDataList2Ctrl(IDC_CLIENT_IMPLEMENTATION_LADDERS, false);
		m_pStatusDate = GetDlgItemUnknown(IDC_STATUS_DATE);
		GetDlgItem(IDC_STATUS_DATE)->EnableWindow(FALSE);

		// (j.gruber 2008-05-22 14:44) - PLID 29437 - added followup date
		m_pFollowupDate = GetDlgItemUnknown(IDC_FOLLOWUP_DATE);
		// (r.gonet 2010-08-13 15:08) - PLID 28871 - m_bFollowupDate was being used in UpdateView before being initialized. 
		//  This was causing auditing when nothing had changed. For some reason the default value in debug mode was true...
		m_bFollowupSet = false;

		// (j.gruber 2007-11-08 16:15) - PLID 28026 - added specialist and status datalists
		//add a <No Selection> Row to the specialist list 
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pSpecialistList->GetNewRow();
		pRow->PutValue(spcID, (long) -1);
		pRow->PutValue(spcName, _variant_t("<No Specialist>"));
		m_pSpecialistList->AddRowSorted(pRow, NULL);

		// (j.gruber 2010-07-29 14:05) - PLID 39878
		//add a <No Selection> Row to the Secondary specialist list 		
		pRow = m_pSecSpecList->GetNewRow();
		pRow->PutValue(spcID, (long) -1);
		pRow->PutValue(spcName, _variant_t("<No Secondary Specialist>"));
		m_pSecSpecList->AddRowSorted(pRow, NULL);


		//add a <No Selection> Row to the Status List
		pRow = m_pStatusList->GetNewRow();
		pRow->PutValue(stcID, (long) -1);
		pRow->PutValue(stcName, _variant_t("<No Status>"));
		m_pStatusList->AddRowSorted(pRow, NULL);

		// (j.gruber 2008-05-22 11:35) - PLID 30144
		pRow = m_pTypeList->GetNewRow();
		pRow->PutValue(tlcID, (long) -1);
		pRow->PutValue(tlcName, _variant_t("<No Type>"));
		m_pTypeList->AddRowSorted(pRow, NULL);

		// (z.manning 2008-09-25 17:44) - PLID 31348 - Make the EMR specialist list read-only if
		// the user doesn't have permission for it.
		// (j.gruber 2010-07-29 15:03) - PLID 39878 - and secondary too
		if(CheckCurrentUserPermissions(bioEmrSpecialist, sptWrite, FALSE, 0, TRUE)) {
			m_pSpecialistList->PutReadOnly(VARIANT_FALSE);
			m_pSecSpecList->PutReadOnly(VARIANT_FALSE);
		}
		else {
			m_pSpecialistList->PutReadOnly(VARIANT_TRUE);
			m_pSecSpecList->PutReadOnly(VARIANT_TRUE);
		}
	}NxCatchAll("Error In CImplementationDlg::OnInitDialog() ");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CImplementationDlg, CPatientDialog)
    //{{AFX_EVENTSINK_MAP(CImplementationDlg)
	ON_EVENT(CImplementationDlg, IDC_EMR_STATUS_LIST, 16 /* SelChosen */, OnSelChosenEmrStatusList, VTS_DISPATCH)
	ON_EVENT(CImplementationDlg, IDC_EMR_STATUS_LIST, 1 /* SelChanging */, OnSelChangingEmrStatusList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CImplementationDlg, IDC_EMR_SPECIALIST_LIST, 1 /* SelChanging */, OnSelChangingEmrSpecialistList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CImplementationDlg, IDC_EMR_SPECIALIST_LIST, 16 /* SelChosen */, OnSelChosenEmrSpecialistList, VTS_DISPATCH)
	ON_EVENT(CImplementationDlg, IDC_EMR_SPECIALIST_LIST, 20 /* TrySetSelFinished */, OnTrySetSelFinishedEmrSpecialistList, VTS_I4 VTS_I4)
	ON_EVENT(CImplementationDlg, IDC_EMR_STATUS_LIST, 20 /* TrySetSelFinished */, OnTrySetSelFinishedEmrStatusList, VTS_I4 VTS_I4)
	ON_EVENT(CImplementationDlg, IDC_CLIENT_IMPLEMENTATION_LADDERS, 19 /* LeftClick */, OnLeftClickClientImplementationLadders, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CImplementationDlg, IDC_CLIENT_IMPLEMENTATION_LADDERS, 7 /* RButtonUp */, OnRButtonUpClientImplementationLadders, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CImplementationDlg, IDC_CLIENT_IMPLEMENTATION_LADDERS, 26 /* RowExpanded */, OnRowExpandedClientImplementationLadders, VTS_DISPATCH)
	ON_EVENT(CImplementationDlg, IDC_CLIENT_IMPLEMENTATION_LADDERS, 27 /* RowCollapsed */, OnRowCollapsedClientImplementationLadders, VTS_DISPATCH)
	ON_EVENT(CImplementationDlg, IDC_CLIENT_IMPLEMENTATION_LADDERS, 3 /* DblClickCell */, OnDblClickCellClientImplementationLadders, VTS_DISPATCH VTS_I2)
	ON_EVENT(CImplementationDlg, IDC_CLIENT_IMPLEMENTATION_LADDERS, 10 /* EditingFinished */, OnEditingFinishedClientImplementationLadders, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CImplementationDlg, IDC_CLIENT_IMPLEMENTATION_LADDERS, 2 /* SelChanged */, OnSelChangedClientImplementationLadders, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CImplementationDlg, IDC_CLIENT_IMPLEMENTATION_LADDERS, 8 /* EditingStarting */, OnEditingStartingClientImplementationLadders, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CImplementationDlg, IDC_EMR_TYPE_LIST, 16 /* SelChosen */, OnSelChosenEmrTypeList, VTS_DISPATCH)
	ON_EVENT(CImplementationDlg, IDC_EMR_TYPE_LIST, 1 /* SelChanging */, OnSelChangingEmrTypeList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CImplementationDlg, IDC_FOLLOWUP_DATE, 1 /* KillFocus */, OnKillFocusFollowupDate, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CImplementationDlg, IDC_SEC_EMR_SPECIALIST_LIST, 1, CImplementationDlg::SelChangingSecEmrSpecialistList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CImplementationDlg, IDC_SEC_EMR_SPECIALIST_LIST, 16, CImplementationDlg::SelChosenSecEmrSpecialistList, VTS_DISPATCH)
	ON_EVENT(CImplementationDlg, IDC_SEC_EMR_SPECIALIST_LIST, 20, CImplementationDlg::TrySetSelFinishedSecEmrSpecialistList, VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

// (j.gruber 2007-11-08 16:15) - PLID 28026 - added specialist and status datalists
void CImplementationDlg::OnSelChosenEmrStatusList(LPDISPATCH lpRow) 
{
	try  {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		
		CString strStatusID;
		CString strStatus;
		if (pRow) {

			long nStatusID = VarLong(pRow->GetValue(stcID), -1);
			strStatus = VarString(pRow->GetValue(stcName), "");

			if (nStatusID == -1) {
				strStatusID = "NULL";
			}
			else {
				strStatusID.Format("%li", nStatusID);
			}
		}
		else {
			strStatusID = "NULL";
		}

		//change the date also
		COleDateTime dt;
		dt = COleDateTime::GetCurrentTime();

		// (j.gruber 2008-02-19 15:26) - PLID 28070 - added status date
		ExecuteSql("UPDATE NxClientsT SET EMRStatusID = %s, EMRStatusDate = '%s' WHERE PersonID = %li", 
			strStatusID, FormatDateTimeForSql(dt), m_nPatientID);

		// (r.gonet 06/09/2010) - PLID 38871 - Audit changes in the Emr Status List
		if(m_strOldEmrStatusID != strStatusID) {
			AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), BeginNewAuditEvent(), aeiImpEmrStatus, m_nPatientID, m_strOldEmrStatus, strStatus, aepMedium);
			m_strOldEmrStatusID = strStatusID;
			m_strOldEmrStatus = strStatus;
		}
	
		// (j.gruber 2008-02-19 15:44) - PLID 28070 - add the date to the box
		m_pStatusDate->SetDateTime(dt);
	
	}NxCatchAll("Error in CImplementationDlg::OnSelChosenEmrStatusList");
	
}

// (j.gruber 2007-11-08 16:15) - PLID 28026 - added specialist and status datalists
void CImplementationDlg::OnSelChangingEmrStatusList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{

	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("CImplementationDlg::OnSelChangingEmrStatusList");
	
	
}

// (j.gruber 2007-11-08 16:15) - PLID 28026 - added specialist and status datalists
void CImplementationDlg::OnSelChangingEmrSpecialistList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("CImplementationDlg::OnSelChangingEmrSpecialistList");
	
	
}

// (j.gruber 2007-11-08 16:15) - PLID 28026 - added specialist and status datalists
void CImplementationDlg::OnSelChosenEmrSpecialistList(LPDISPATCH lpRow) 
{
	try  {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		
		CString strSpecialistID;
		CString strSpecialistName;
		if (pRow) {

			long nSpecialistID = VarLong(pRow->GetValue(spcID), -1);
			strSpecialistName = VarString(pRow->GetValue(spcName), "");

			if (nSpecialistID == -1) {
				strSpecialistID = "NULL";
			}
			else {
				strSpecialistID.Format("%li", nSpecialistID);
			}
		}
		else {
			strSpecialistID = "NULL";
		}

		ExecuteSql("UPDATE NxClientsT SET EMRSpecialistID = %s WHERE PersonID = %li", 
			strSpecialistID, m_nPatientID);

		// (r.gonet 06/09/2010) - PLID 38871 - Audit changes in the Emr Specialist list
		if(m_strOldEmrSpecialistID != strSpecialistID) {
			AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), BeginNewAuditEvent(), aeiImpEmrSpecialist, m_nPatientID, m_strOldEmrSpecialist, strSpecialistName, aepMedium);
			m_strOldEmrSpecialistID = strSpecialistID;
			m_strOldEmrSpecialist = strSpecialistName;
		}
	
	
	}NxCatchAll(" CImplementationDlg::OnSelChosenEmrSpecialistList");
	
}



CString CImplementationDlg::GenerateActionText(long nActionID, CDWordArray *dwActionIDs, CStringArray *strActionPaths) {

	try {
		
		if (dwActionIDs->GetSize() > 0) {
			ADODB::_RecordsetPtr rs;

			CString strIDs;
			for (int i = 0; i < dwActionIDs->GetSize(); i++) {
				strIDs += AsString((long)dwActionIDs->GetAt(i)) + ",";
			}
			strIDs.TrimRight(",");

			CString strBase;

			switch (nActionID) {

				case atCompleteEMR:
					strBase = "Complete EMR: ";
					rs = CreateRecordset("SELECT ID, Name as Description FROM EMRTemplateT WHERE ID IN (%s)", strIDs);
				break;

				case atMergePacket:
					strBase = "Merge Packet: ";
					rs = CreateRecordset("SELECT ID, Name as Description FROM PacketsT WHERE Deleted = 0 and ProcedureRelated  = 0 AND ID IN (%s)", strIDs);
				break;
				case atAptByCategory:
					strBase = "Appt By Category: ";
					rs = CreateRecordset("SELECT * FROM ( "
						" SELECT 1 as ID, 'Consult' as Description  "
						" UNION  "
						" SELECT 2, 'PreOp' "
						" UNION "
						" SELECT 3, 'Minor Procedure' "
						" UNION "
						" SELECT 4, 'Surgery' "
						" UNION  "
						" SELECT 5, 'Follow-up' "
						" UNION  "
						" SELECT 6, 'Other Procedure') Q WHERE ID IN (%s)", strIDs);
				break;
				case atAptByType:
					strBase = "Appt By Type: ";
					rs = CreateRecordset("SELECT ID, Name as Description FROM AptTypeT WHERE InActive = 0 AND ID IN (%s)", strIDs);
				break;
			}

			CString strText;
			while (!rs->eof) {

				strText += AdoFldString(rs, "Description") + ", ";

				rs->MoveNext();
			}

			strText.TrimRight(", ");

			return strBase + " " + strText;
		}
		else if (strActionPaths->GetSize()) {

			CString strText;
			CString strBase;

			switch (nActionID) {
				case atEmailDocument:
					strBase = "Email Document: ";
				break;

				case atMergeDocument:
					strBase = "Merge Document: ";
				break;
			}

			for (int i = 0; i < strActionPaths->GetSize(); i++) {

				strText += strActionPaths->GetAt(i) + ";";
			}

			strText.TrimRight("; ");

			return strBase + " " + strText;
		}
		else {

			//must be a template or procedure
			if (nActionID == atSelectProcedures) {
				return "EMR Procedures";
			}
			else if (nActionID == atCustomizeTemplates) {
				return "Customize Templates";
			}
			else if (nActionID == atCustom) {
				// (j.gruber 2008-05-22 12:46) - PLID 29445 - adding custom step
				return "Custom Step - <No Action>";
			}
			else {
				ASSERT(FALSE);
			}
		}


	}NxCatchAll("Error in CImplementationDlg::GenerateActionText");
	return "";

}


void CImplementationDlg::LoadLadderList(BOOL bKeepSelectedRow /*=TRUE*/) 
{
	try {

		long nOrigLadderID = -1, nOrigStepID = -1;
		if (bKeepSelectedRow) {
			//get the currently selected items in the list
			NXDATALIST2Lib::IRowSettingsPtr pOrigRow;
			pOrigRow = m_pLadderList->GetCurSel();
			if (pOrigRow) {
				nOrigLadderID = VarLong(pOrigRow->GetValue(lcLadderID), -1);
				nOrigStepID = VarLong(pOrigRow->GetValue(lcStepID), -1);
			}
		}

		

		m_pLadderList->Clear();
		
		ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT CILT.ID as LadderID, CILT.Name as LadderName, CILT.Done as LadderDone, CILT.TypeID, "
			" CIST.ID as StepID, CIST.SortOrder, CIST.Active, CIST.ActiveDate, CIST.DocumentCategoryID, CIST.IsClientAssigned, CIST.ActionID, CIST.StepNAme, CIST.Note, CIST.ClientDone, CIST.Done as StepDone, CISCT.ActionItemID, CISCT.ActionItemPath "
			" FROM ClientImplementationLadderT CILT LEFT JOIN ClientImplementationStepsT CIST ON "
			" CILT.ID = CIST.LadderID "
			" LEFT JOIN ClientImplementationStepCriteriaT CISCT ON CIST.ID = CISCT.StepID "
			" WHERE CILT.ClientID = {INT} "
			" ORDER BY CILT.ID, CIST.SortOrder ", m_nPatientID);


		long nOldLadderID = -1;
		long nLadderID;
		long nOldStepID = -1;
		long nStepID;
		NXDATALIST2Lib::IRowSettingsPtr pParentRow;
		NXDATALIST2Lib::IRowSettingsPtr pRow;

		_variant_t varTrue(VARIANT_TRUE, VT_BOOL);
		_variant_t varFalse(VARIANT_FALSE, VT_BOOL);

		CDWordArray dwActionIDs;
		CStringArray strActionPaths;

		long nActionID = -1;
		long nTypeID;
		while (!rs->eof) {

			nLadderID = AdoFldLong(rs, "LadderID");
			nTypeID = AdoFldLong(rs, "TypeID");

			_variant_t varNULL;
			varNULL.vt = VT_NULL;

			if (nOldLadderID != nLadderID) {

				//add a parent row
				pParentRow = m_pLadderList->GetNewRow();

				pParentRow->PutValue(lcLadderID, nLadderID);
				pParentRow->PutValue(lcLadderTypeID, nTypeID);
				pParentRow->PutValue(lcStepID, varNULL);
				pParentRow->PutValue(lcSortOrder, varNULL);
				pParentRow->PutValue(lcActive, varNULL);
				pParentRow->PutValue(lcActiveDate, varNULL);
				pParentRow->PutValue(lcLadderName, _variant_t(AdoFldString(rs, "LadderName", "")));
				pParentRow->PutValue(lcNotes, varNULL);
				pParentRow->PutValue(lcActionID, varNULL);
				pParentRow->PutValue(lcAction, varNULL);
				pParentRow->PutValue(lcDocCategoryID, varNULL);
				pParentRow->PutValue(lcDocAssigned, varNULL);
				pParentRow->PutValue(lcDocDone, varNULL);
				if (AdoFldBool(rs, "LadderDone", false)) {
					pParentRow->PutValue(lcDone, varTrue);
				}
				else {
					pParentRow->PutValue(lcDone, varFalse);
				}

				m_pLadderList->AddRowSorted(pParentRow, NULL);

				nOldLadderID = nLadderID;
				
				

			}

			nStepID = AdoFldLong(rs, "StepID");
			nActionID = AdoFldLong(rs, "ActionID");
		
			if (nOldStepID != nStepID) {

				if (nOldStepID != -1) {
					long nTempActionID = VarLong(pRow->GetValue(lcActionID));
					CString strActionText = GenerateActionText(nTempActionID, &dwActionIDs, &strActionPaths);
					dwActionIDs.RemoveAll();
					strActionPaths.RemoveAll();
					pRow->PutValue(lcAction, _variant_t(strActionText));
				}

				BOOL bIsClientAssigned = AdoFldBool(rs, "IsClientAssigned", FALSE);

				pRow = m_pLadderList->GetNewRow();
				pRow->PutValue(lcLadderID, nLadderID);
				pRow->PutValue(lcStepID, nStepID);
				if (AdoFldBool(rs, "Active", false)) {
					pRow->PutValue(lcActive, varTrue);
				}
				else {
					pRow->PutValue(lcActive, varFalse);
				}
				pRow->PutValue(lcActionID, AdoFldLong(rs, "ActionID"));
				_variant_t varDate = rs->Fields->Item["ActiveDate"]->Value;
				if (varDate.vt == VT_DATE) {
					pRow->PutValue(lcActiveDate, varDate);
					
					//check to see if this step should be active
					COleDateTime dtActive = VarDateTime(varDate);
					if (dtActive <= COleDateTime::GetCurrentTime()) {
						if (!AdoFldBool(rs, "Active", false) && !AdoFldBool(rs, "StepDone", false)) {
							//activate the step
							pRow->PutValue(lcActive, varTrue);
							ExecuteParamSql("UPDATE ClientImplementationStepsT SET Active = 1 WHERE ID = {INT}", nStepID);
						}
					}
				}
				pRow->PutValue(lcSortOrder, AdoFldLong(rs, "SortOrder"));
				pRow->PutValue(lcLadderName, _variant_t(AdoFldString(rs, "StepName", "")));
				pRow->PutValue(lcNotes, _variant_t(AdoFldString(rs, "Note", "")));
				pRow->PutValue(lcDocCategoryID, AdoFldLong(rs, "DocumentCategoryID", -1));
				BOOL bIsClientDone = AdoFldBool(rs, "ClientDone", FALSE);
				BOOL bIsDone = AdoFldBool(rs, "StepDone", false);
				if (bIsClientAssigned) {
					pRow->PutValue(lcDocAssigned, varTrue);
				}
				else {
					pRow->PutValue(lcDocAssigned, varFalse);
				}
				if (bIsClientDone) {
					pRow->PutValue(lcDocDone, varTrue);
				}
				else {
					pRow->PutValue(lcDocDone, varFalse);				
				}
				if (bIsDone) {
					pRow->PutValue(lcDone, varTrue);
				}
				else {
					pRow->PutValue(lcDone, varFalse);
				}


				m_pLadderList->AddRowSorted(pRow, pParentRow);

				nOldStepID = nStepID;

				//colors
				if (!VarBool(pRow->GetValue(lcActive))) {
					if (VarBool(pRow->GetValue(lcDone))) {
						pRow->PutBackColor(EMR_SPEC_DONE);
					}
					else {
						//its not active yet
						pRow->PutBackColor(EMR_SPEC_NOT_ACTIVE);
					}
				}
				else {
					if (VarBool(pRow->GetValue(lcDone))) {
						//its done
						pRow->PutBackColor(EMR_SPEC_DONE);
					}
					else {
						//in progress
						pRow->PutBackColor(EMR_SPEC_IN_PROGRESS);
					}
				}
						
			}

			

			
			//add the action to an array to be parsed later
			switch (nActionID) {

				case atEmailDocument:
				case atMergeDocument:
					strActionPaths.Add(AdoFldString(rs, "ActionItemPath", ""));
				break;


				case atSelectProcedures:
				case atCustomizeTemplates:
				case atCustom:
				break;
				
				case atCompleteEMR:
				case atMergePacket:
				case atAptByCategory:
				case atAptByType:
					dwActionIDs.Add(AdoFldLong(rs, "ActionItemID", -1));
				break;
			}

			rs->MoveNext();
		}

		//add the last action
		if (nActionID > 0) {
			CString strActionText = GenerateActionText(nActionID, &dwActionIDs, &strActionPaths);
			pRow->PutValue(lcAction, _variant_t(strActionText));											
		}

		//now go back through and expand any item that was expanded
		for (int i =0; i < m_dwExpandedLadders.GetSize(); i++) {
			long nLadderID = m_dwExpandedLadders.GetAt(i);

			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pLadderList->FindByColumn(lcLadderID, nLadderID, NULL, FALSE);

			if (pRow) {
				pRow->PutExpanded(VARIANT_TRUE);
			}
		}


		//if they want to, put the selection back
		if (bKeepSelectedRow) {

			if (nOrigStepID != -1) {
				m_pLadderList->SetSelByColumn(lcStepID, nOrigStepID);
			}
			else {
				if (nOrigLadderID != -1) {
					m_pLadderList->SetSelByColumn(lcLadderID, nOrigLadderID);
				}
			}
		}

	}NxCatchAll("CImplementationDlg::LoadLadderList");

}

void CImplementationDlg::Load() {

	try {

		// (j.gruber 2008-02-19 15:23) - PLID 28070 - added status note and date to the query
		// (j.gruber 2008-05-22 14:46) - PLID 29437 - added followup date to the query
		// (j.gruber 2008-12-22 09:25) - PLID 28071 - added client picture, website, point person, point person email
		// (j.gruber 2010-07-29 14:51) - PLID 39878 - Secondary Specialist
		ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT PatientsT.ImageIndex, EmrSpecialistID, EMRStatusID, EMRPointPerson, EMRPointPersonEmail, PMPointPerson, PMPointPersonEmail, (SELECT TextParam FROM CustomFieldDataT WHERE PersonID = {INT} and FieldID = 3) AS ClientWebsite, NxClientsT.EMRStatusNote, NxClientsT.EMRStatusDate, NxClientsT.SecondaryEMRSpecialistID, "
			" NxClientsT.EMRTypeID, NxClientsT.EMRFollowupDate "
			" FROM NxClientsT INNER JOIN PersonT ON NxClientsT.PersonID = PersonT.ID "
			" INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID" 			
			" WHERE NxClientsT.PersonID = {INT}", m_nPatientID, m_nPatientID);

		if (!rs->eof) {

			//there aren't any inactives here, so this should either set it or throw the event which we will handle then
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_pSpecialistList->TrySetSelByColumn_Deprecated(spcID, AdoFldLong(rs, "EMRSpecialistID", -1));
						
			// (j.gruber 2010-07-29 14:51) - PLID 39878
			long nSecSpecID = AdoFldLong(rs, "SecondaryEMRSpecialistID", -1);
			m_pSecSpecList->TrySetSelByColumn_Deprecated(spcID, nSecSpecID);
						
			//same here
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_pStatusList->TrySetSelByColumn_Deprecated(stcID, AdoFldLong(rs, "EMRStatusID", -1));

			// (j.gruber 2008-05-22 11:36) - PLID 30144
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_pTypeList->TrySetSelByColumn_Deprecated(tlcID, AdoFldLong(rs, "EMRTypeID", -1));

			// (j.gruber 2008-12-22 09:25) - PLID 28071 - website
			// (j.gruber 2007-12-11 17:10) - PLID 28329 - made a go button for the website field
			CString strWebsite = AdoFldString(rs, "ClientWebsite", "");
			if (strWebsite.IsEmpty()) {
				SetDlgItemText(IDC_CLIENT_WEBSITE, "");
				GetDlgItem(IDC_GO_WEBSITE)->EnableWindow(FALSE);
			}
			else {
				SetDlgItemText(IDC_CLIENT_WEBSITE, strWebsite);
				GetDlgItem(IDC_GO_WEBSITE)->EnableWindow(TRUE);
			}

			
			// (j.gruber 2008-12-22 09:25) - PLID 28071 - added client picture, website, point person, point person email
			SetDlgItemText(IDC_EMR_POINT_PERSON, AdoFldString(rs, "EMRPointPerson", ""));
			SetDlgItemText(IDC_EMR_POINT_EMAIL, AdoFldString(rs, "EMRPointPersonEmail", ""));
			SetDlgItemText(IDC_PM_POINT_PERSON, AdoFldString(rs, "PMPointPerson", ""));
			SetDlgItemText(IDC_PM_POINT_EMAIL, AdoFldString(rs, "PMPointPersonEmail", ""));

			m_strEMRStatus = AdoFldString(rs, "EMRStatusNote", "");
			SetDlgItemText(IDC_STATUS_NOTE, m_strEMRStatus);
			
			COleDateTime dtNULL, dtStatus;
			dtNULL.SetDate(1899,12,30);
			dtStatus = AdoFldDateTime(rs, "EmrStatusDate", dtNULL);
			if (dtStatus != dtNULL) {
				m_pStatusDate->SetDateTime(dtStatus);
			}	
			else {
				m_pStatusDate->Clear();
			}

			// (j.gruber 2008-05-22 14:45) - PLID 29437 - added followup date
			COleDateTime dtFollowup = AdoFldDateTime(rs, "EMRFollowupDate", dtNULL);
			if (dtFollowup != dtNULL) {
				m_pFollowupDate->SetDateTime(dtFollowup);
				m_dtFollowup = dtFollowup;
				m_bFollowupSet = true;
			}
			else {
				m_pFollowupDate->Clear();
				m_bFollowupSet = false;
			}

			GetDlgItem(IDC_EMR_SPECIALIST_LIST)->EnableWindow(TRUE);
			GetDlgItem(IDC_SEC_EMR_SPECIALIST_LIST)->EnableWindow(TRUE); // (j.gruber 2010-07-29 14:52) - PLID 39878
			GetDlgItem(IDC_EMR_STATUS_LIST)->EnableWindow(TRUE);

			GetDlgItem(IDC_DELETE_IMPLEMENTATION_LADDER)->EnableWindow(TRUE);
			GetDlgItem(IDC_ADD_IMPLEMENTATION_LADDER)->EnableWindow(TRUE);
			GetDlgItem(IDC_CLIENT_IMPLEMENTATION_LADDERS)->EnableWindow(TRUE);


			GetDlgItem(IDC_CLIENT_WEBSITE)->EnableWindow(TRUE);
			GetDlgItem(IDC_EMR_POINT_PERSON)->EnableWindow(TRUE);
			GetDlgItem(IDC_EMR_POINT_EMAIL)->EnableWindow(TRUE);
			GetDlgItem(IDC_PM_POINT_PERSON)->EnableWindow(TRUE);
			GetDlgItem(IDC_PM_POINT_EMAIL)->EnableWindow(TRUE);
			GetDlgItem(IDC_STATUS_NOTE)->EnableWindow(TRUE);

			//load the button
			if (GetPatientAttachedImageCount(m_nPatientID) > 0 ) {
				m_btnImage.ShowWindow(SW_SHOW);
				m_btnImage.m_source = eImageSrcPractice;
				long nImageIndex = AdoFldLong(rs, "ImageIndex", 0);
				m_btnImage.m_image = LoadPatientAttachedImage(m_nPatientID, nImageIndex, &m_btnImage.m_progress, m_btnImage.m_strPracticeFileName);
			}
			else {
				m_btnImage.ShowWindow(SW_HIDE);
			}

			LoadLadderList(FALSE);
			// (j.gruber 2010-07-29 14:17) - PLID 39878
			m_strOldSecEmrSpecID = AsString(GetEmrSecSpecialistID());
			m_strOldSecEmrSpec = GetEmrSecSpecialist();
		}
		else {
			//they aren't clients, so they don't have this information yet
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_pSpecialistList->TrySetSelByColumn_Deprecated(spcID, (long)-1);
			m_pSecSpecList->TrySetSelByColumn_Deprecated(spcID, (long)-1); //39878
			m_pStatusList->TrySetSelByColumn_Deprecated(stcID, (long)-1);
			m_pTypeList->TrySetSelByColumn_Deprecated(tlcID, (long)-1);
			
			//grey out the boxes as well, since these values can't be set yet
			GetDlgItem(IDC_EMR_SPECIALIST_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_SEC_EMR_SPECIALIST_LIST)->EnableWindow(FALSE); //39878
			GetDlgItem(IDC_EMR_STATUS_LIST)->EnableWindow(FALSE);
			

			GetDlgItem(IDC_CLIENT_WEBSITE)->EnableWindow(FALSE);
			SetDlgItemText(IDC_CLIENT_WEBSITE, "");
			GetDlgItem(IDC_EMR_POINT_PERSON)->EnableWindow(FALSE);
			SetDlgItemText(IDC_EMR_POINT_PERSON, "");
			GetDlgItem(IDC_EMR_POINT_EMAIL)->EnableWindow(FALSE);
			SetDlgItemText(IDC_EMR_POINT_EMAIL, "");
			GetDlgItem(IDC_PM_POINT_PERSON)->EnableWindow(FALSE);
			SetDlgItemText(IDC_PM_POINT_PERSON, "");
			GetDlgItem(IDC_PM_POINT_EMAIL)->EnableWindow(FALSE);
			SetDlgItemText(IDC_PM_POINT_EMAIL, "");
			GetDlgItem(IDC_STATUS_NOTE)->EnableWindow(FALSE);
			SetDlgItemText(IDC_STATUS_NOTE, "");
			m_pStatusDate->Clear();
			m_pFollowupDate->Clear();
			m_bFollowupSet = false;
			// (j.gruber 2007-12-11 17:10) - PLID 28329 - made a go button for the website field
			GetDlgItem(IDC_GO_WEBSITE)->EnableWindow(FALSE); 

			GetDlgItem(IDC_DELETE_IMPLEMENTATION_LADDER)->EnableWindow(FALSE);
			GetDlgItem(IDC_ADD_IMPLEMENTATION_LADDER)->EnableWindow(FALSE);
			GetDlgItem(IDC_CLIENT_IMPLEMENTATION_LADDERS)->EnableWindow(FALSE);
		}

		// (r.gonet 06/09/2010) - PLID 38871 - Assign the old audit values to what is loaded
		//  These should really be filled for clients AND prospects, not just clients 
		m_strOldEmrSpecialistID = AsString(GetEmrSpecialistID());
		m_strOldEmrStatusID = AsString(GetEmrStatusID());
		m_strOldEmrTypeID = AsString(GetEmrTypeID());
		m_strOldEmrSpecialist = GetEmrSpecialist();
		m_strOldEmrStatus = GetEmrStatus();
		m_strOldEmrType = GetEmrType();
		GetDlgItemText(IDC_STATUS_NOTE, m_strOldStatusNote);
		GetDlgItemText(IDC_EMR_POINT_PERSON, m_strOldEmrClientPointPerson);
		GetDlgItemText(IDC_EMR_POINT_EMAIL, m_strOldEmrPointPersonEmail);
		GetDlgItemText(IDC_PM_POINT_PERSON, m_strOldPMClientPointPerson);
		GetDlgItemText(IDC_PM_POINT_EMAIL, m_strOldPMPointPersonEmail);

		
	}NxCatchAll("Error in CImplementationDlg::Load()");
	
}



// (j.gruber 2007-11-08 16:15) - PLID 28026 - added specialist and status datalists
void CImplementationDlg::OnTrySetSelFinishedEmrSpecialistList(long nRowEnum, long nFlags) 
{
	try {
		if(nFlags == NXDATALIST2Lib::dlTrySetSelFinishedFailure) {
			//this really shouldn't happen since we have no inactives, everything should be accounted for
			ASSERT(FALSE);
		}
	}NxCatchAll("Error in CImplementationDlg::OnTrySetSelFinishedEmrSpecialistList");
	
}

// (j.gruber 2007-11-08 16:15) - PLID 28026 - added specialist and status datalists
void CImplementationDlg::OnTrySetSelFinishedEmrStatusList(long nRowEnum, long nFlags) 
{
	try {
		if(nFlags == NXDATALIST2Lib::dlTrySetSelFinishedFailure) {
			//this really shouldn't happen since we have no inactives, everything should be accounted for
			ASSERT(FALSE);
		}
	}NxCatchAll("Error in CImplementationDlg::OnTrySetSelFinishedEmrStatusList");
	
}


void CImplementationDlg::Save(long nID) {

	switch (nID) {

		// (j.gruber 2008-12-22 09:25) - PLID 28071 - added client picture, website, point person, point person email
		case IDC_EMR_POINT_PERSON:
			{
				CString strEmrPointPerson;
				GetDlgItemText(IDC_EMR_POINT_PERSON, strEmrPointPerson);

				ExecuteParamSql("UPDATE NxClientsT SET EMRPointPerson = {STRING} WHERE PersonID = {INT}", strEmrPointPerson, m_nPatientID);
			
				// (r.gonet 06/21/2010) - PLID 38871 - Audit point person change
				if(m_strOldEmrClientPointPerson != strEmrPointPerson) {
					AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), BeginNewAuditEvent(), aeiImpEmrClientPointPerson, m_nPatientID, m_strOldEmrClientPointPerson, strEmrPointPerson, aepMedium);
					m_strOldEmrClientPointPerson = strEmrPointPerson;
				}
			}
		break;
		case IDC_EMR_POINT_EMAIL:
			{
				CString strEmrPointPersonEmail;
				GetDlgItemText(IDC_EMR_POINT_EMAIL, strEmrPointPersonEmail);
	
				ExecuteParamSql("UPDATE NxClientsT SET EMRPointPersonEmail = {STRING} WHERE PersonID = {INT} ", strEmrPointPersonEmail, m_nPatientID);
			
				// (r.gonet 06/09/2010) - PLID 38871 - Audit changes to the EMR Point Person's email
				if(m_strOldEmrPointPersonEmail != strEmrPointPersonEmail) {
					AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), BeginNewAuditEvent(), aeiImpEmrPointPersonEmail, m_nPatientID, m_strOldEmrPointPersonEmail, strEmrPointPersonEmail, aepMedium);
					m_strOldEmrPointPersonEmail = strEmrPointPersonEmail;
				}
			}
		break;
		case IDC_PM_POINT_EMAIL:
			{
				CString strPMPointPersonEmail;
				GetDlgItemText(IDC_PM_POINT_EMAIL, strPMPointPersonEmail);

				ExecuteParamSql("UPDATE NxClientsT SET PMPointPersonEmail = {STRING} WHERE PersonID = {INT}", strPMPointPersonEmail, m_nPatientID);
			
				// (r.gonet 06/09/2010) - PLID 38871 - Audit changes to the PM point person's email
				if(m_strOldPMPointPersonEmail != strPMPointPersonEmail) {
					AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), BeginNewAuditEvent(), aeiImpPMPointPersonEmail, m_nPatientID, m_strOldPMPointPersonEmail, strPMPointPersonEmail, aepMedium);
					m_strOldPMPointPersonEmail = strPMPointPersonEmail;
				}
			}
		break;
		case IDC_PM_POINT_PERSON:
			{
				CString strPMPointPerson;
				GetDlgItemText(IDC_PM_POINT_PERSON, strPMPointPerson);

				ExecuteParamSql("UPDATE NxClientsT SET PMPointPerson = {STRING} WHERE PersonID = {INT} ", strPMPointPerson, m_nPatientID);
			
				// (r.gonet 06/09/2010) - PLID 38871 - Edit changes to the PM point person
				if(m_strOldPMClientPointPerson != strPMPointPerson) {
					AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), BeginNewAuditEvent(), aeiImpPMClientPointPerson, m_nPatientID, m_strOldPMClientPointPerson, strPMPointPerson, aepMedium);
					m_strOldPMClientPointPerson = strPMPointPerson;
				}
			}
		break;
		// (j.gruber 2008-02-19 15:23) - PLID 28070 - added
		case IDC_STATUS_NOTE:
			{
				CString strStatusNote;
				GetDlgItemText(IDC_STATUS_NOTE, strStatusNote);

				//check for case changes too
				if (strStatusNote != m_strEMRStatus) {

					//its changed, we need to change the date as well
					COleDateTime dt;
					dt = COleDateTime::GetCurrentTime();

					// (j.gruber 2008-02-19 15:27) - PLID 28070 - added status date
					ExecuteSql("UPDATE NxClientsT SET EMRStatusNote = '%s', EMRStatusDate = '%s' WHERE PersonID = %li",
						_Q(strStatusNote), FormatDateTimeForSql(dt), m_nPatientID);

					// (r.gonet 06/09/2010) - PLID 38871 - Audit changes to the status note
					AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), BeginNewAuditEvent(), aeiImpStatusNote, m_nPatientID, m_strOldStatusNote, strStatusNote, aepMedium);
					m_strOldStatusNote = strStatusNote;

					m_pStatusDate->SetDateTime(dt);
					m_strEMRStatus = strStatusNote;
				}

			}
		break;

		case IDC_FOLLOWUP_DATE:
			{
				COleDateTime dtMin, dtFollowup;
				dtFollowup = m_pFollowupDate->GetDateTime();
				dtMin.SetDate(1800,12,31);

				if (m_pFollowupDate->GetStatus() == 3) {
					CString strOldFollowupDate = FormatDateTimeForSql(m_dtFollowup);
					
					ExecuteSql("UPDATE NxClientsT SET EMRFollowupDate = NULL WHERE PersonID = %li", 
						m_nPatientID);

					// (r.gonet 08/13/2010) - PLID 38871 - Audit that the Followup date was cleared out. It must have been set first though.
					//  It will be false when the user switches between patients.
					if(m_bFollowupSet) {
						AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), BeginNewAuditEvent(), aeiImpFollowupDate, m_nPatientID, strOldFollowupDate, "", aepMedium);
					}
					m_bFollowupSet = false;	
				}
				else {
					if (dtFollowup < dtMin || dtFollowup.GetStatus() != COleDateTime::valid) {
						AfxMessageBox("Please enter a valid date");
						m_pFollowupDate->SetDateTime(m_dtFollowup);
					}
					else {
						ExecuteSql("UPDATE NxClientsT SET EMRFollowupDate = '%s' WHERE PersonID = %li", 
							FormatDateTimeForSql(dtFollowup), m_nPatientID);
						
						CString strOldFollowup = (m_bFollowupSet ? FormatDateTimeForSql(m_dtFollowup) : "");
						CString strFollowup = FormatDateTimeForSql(dtFollowup);

						
						m_dtFollowup = dtFollowup;
						m_bFollowupSet = true;

						// (r.gonet 06/09/2010) - PLID 38871 - Audit changes to the Emr Followup date
						AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), BeginNewAuditEvent(), aeiImpFollowupDate, m_nPatientID, strOldFollowup, strFollowup, aepMedium);
					}
				}
			}
		break;
	}



}

void CImplementationDlg::StoreDetails() {

	try {
		if((m_bFollowupSet && m_pFollowupDate->GetStatus() == 3) || 
			((long)m_pFollowupDate->GetDateTime() != (long)m_dtFollowup.m_dt) 
			|| (!m_bFollowupSet && m_pFollowupDate->GetStatus() == 1)) {
			Save(IDC_FOLLOWUP_DATE);
		}

		if(GetFocus()) {
			Save (GetFocus()->GetDlgCtrlID());
		}

	}NxCatchAll("Error in StoreDetails");





}

void CImplementationDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh 
{

	try {

		StoreDetails();

		m_nPatientID = GetActivePatientID();

		//check focus
		/*CWnd* pwndFocus = GetFocus();
		if(pwndFocus && ::IsWindow(pwndFocus->GetSafeHwnd())) {
			::SetFocus(NULL);
			pwndFocus->SetFocus();
		}*/

		m_dwExpandedLadders.RemoveAll();
		Load();
	}NxCatchAll("Error in UpdateView");
}

void CImplementationDlg::SetColor(OLE_COLOR nNewColor)
{
	try {
		((CNxColor*)GetDlgItem(IDC_IMPLEMENTATION_COLOR))->SetColor(nNewColor);
	
		CPatientDialog::SetColor(nNewColor);
	}NxCatchAll("Error in CImplementationDlg::SetColor");
}


void CImplementationDlg::EmailDocuments(long nStepID, long nTypeID) {

	try {

		ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT ActionItemPath FROM ClientImplementationStepCriteriaT WHERE StepID = {INT}", nStepID);

		CStringArray strPaths;

		while (! rs->eof) {

			strPaths.Add(AdoFldString(rs, "ActionItemPath"));

			rs->MoveNext();
		}

		//these types are hard coded into the database at the moment
		CString strEmail;
		switch (nTypeID) {

			case 1: //EMR
				GetDlgItemText(IDC_EMR_POINT_EMAIL, strEmail);
			break;

			case 2: //PM
				GetDlgItemText(IDC_PM_POINT_EMAIL, strEmail);
			break;

			default:
				//the only thing left is sales and they don't have an point person on here atm
				strEmail = "";
			break;
		}

		SendEmail(this, strEmail, "", "", strPaths);

	}NxCatchAll("Error in CImplementationDlg::EmailDocuments() ");


}

void CImplementationDlg::MergeDocument(CString strTemplateName, long nDocCategoryID) {


	try {

		CMergeEngine mi;
		mi.m_nPicID = -1;
			
		
		CWaitCursor wc;
				
		/// Generate the temp table
		CString strSql;
		strSql.Format("SELECT ID FROM PersonT WHERE ID = %li", m_nPatientID);
		CString strMergeT = CreateTempIDTable(strSql, "ID");
			
		// Merge
		if (g_bMergeAllFields) mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;
		mi.m_nFlags |= BMS_SAVE_FILE_AND_HISTORY;
		
		CString strName;

		ADODB::_RecordsetPtr rs = CreateRecordset("SELECT First, Middle, Last, Title, Email FROM PersonT WHERE ID = %li",GetCurrentUserID());
		if(!rs->eof) {
			mi.m_strSenderFirst = AdoFldString(rs, "First","");
			mi.m_strSenderMiddle = AdoFldString(rs, "Middle","");
			mi.m_strSenderLast = AdoFldString(rs, "Last","");
			strName = mi.m_strSenderFirst + (mi.m_strSenderMiddle.IsEmpty() ? "" : (" "+ mi.m_strSenderMiddle)) + " " + mi.m_strSenderLast;
			strName.TrimRight();
			mi.m_strSender = strName;
			mi.m_strSenderTitle = AdoFldString(rs, "Title","");
			mi.m_strSenderEmail = AdoFldString(rs, "Email","");
		}
		rs->Close();
		
		//assign a category ID
		if (nDocCategoryID != -1) {
			mi.m_nCategoryID = nDocCategoryID;
		}
					
		// Do the merge
		mi.MergeToWord(strTemplateName, std::vector<CString>(), strMergeT);

	}NxCatchAll("Error in CImplementationDlg::MergeDocument");

}


void CImplementationDlg::MergePacket(long nStepID, long nDocumentCategoryID) {

	try {

		ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT ActionItemID FROM ClientImplementationStepCriteriaT WHERE StepID = {INT}", nStepID);

		if (! rs->eof) {
			long nPacketID = AdoFldLong(rs, "ActionItemID");

			CStringArray arystrPaths;

			
			long nPacketCategoryID = nDocumentCategoryID;

			// Loop through each template, adding the full path of each one to the array
			ADODB::_RecordsetPtr rsTemplates = CreateRecordset("SELECT Path FROM MergeTemplatesT INNER JOIN PacketComponentsT ON MergeTemplatesT.ID = PacketComponentsT.MergeTemplateID WHERE PacketComponentsT.PacketID = %li ", nPacketID);
			CStringArray saPaths;
			while(!rsTemplates->eof) {
				saPaths.Add(AdoFldString(rsTemplates, "Path"));
				rsTemplates->MoveNext();
			}
			rsTemplates->Close();

			for(int nTemplate = 0; nTemplate < saPaths.GetSize(); nTemplate++) {
				CMergeEngine mi;
				mi.m_nPicID = -1;
				
				CString strPath = saPaths.GetAt(nTemplate);
					
				//if it starts with a drive letter or "\\" then it's an absolute path, otherwise, it's relative to the shared path.
				if(strPath.GetAt(0) == '\\' && strPath.GetAt(1) != '\\') {
					strPath = GetSharedPath() ^ strPath;
				}


				// PLID  15371: make sure the path exists and if it doesn't, output a warning
				if (! DoesExist(strPath)) {
					CString strMessage;
					strMessage.Format("The file %s in the packet does not exist.\nPlease check the path of the template. This packet cannot be merged.", strPath);
					MsgBox(strMessage);
					return;
				}
					
				/// Generate the temp table
				CString strSql;
				strSql.Format("SELECT ID FROM PersonT WHERE ID = %li", m_nPatientID);
				CString strMergeTo = CreateTempIDTable(strSql, "ID");
					
				// Merge
				if (g_bMergeAllFields) mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;
				mi.m_nFlags |= BMS_SAVE_FILE_AND_HISTORY;

				// (z.manning, 03/05/2008) - PLID 29131 - Now have a function to load sender information.
				mi.LoadSenderInfo(FALSE);

				//assign a category ID
				if(nDocumentCategoryID != -1) {
					mi.m_nCategoryID = nDocumentCategoryID;
				}

				// Do the merge
				//TES 8/10/2004: We still want to store that the packet was merged.
				long nMergedPacketID = NewNumber("MergedPacketsT", "ID");
				ExecuteSql("INSERT INTO MergedPacketsT (ID, PacketID, SeparateDocuments) VALUES (%li, %li, 1)", nMergedPacketID, nPacketID);

				CString strCurrentTemplateFilePath;
				// (c.haag 2016-02-23) - PLID 68416 - We no longer catch Word-specific exceptions here. Those are now managed deep within the WordProcessor application object
				// (z.manning 2016-06-14 8:11) - NX-100806 - Check the return value and break out of the loop if it fails
				if (!mi.MergeToWord(strPath, std::vector<CString>(), strMergeTo, "", nMergedPacketID, nPacketCategoryID, true, FormatString("Template %i of %i", nTemplate + 1, saPaths.GetSize()))) {
					break;
				}
			}			
		}				

	}NxCatchAll("Error in CImplementationDlg MergePacket");


}

void CImplementationDlg::OnLeftClickClientImplementationLadders(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			NXDATALIST2Lib::IRowSettingsPtr pParentRow;
			pParentRow = pRow->GetParentRow();
			if (pParentRow) {
				

				switch (nCol) {

					case lcAction:
						{

						long nStepID = VarLong(pRow->GetValue(lcStepID));
						long nTypeID = VarLong(pParentRow->GetValue(lcLadderTypeID));
						long nDocumentCategoryID = VarLong(pRow->GetValue(lcDocCategoryID), -1);			
						long nLadderID = VarLong(pRow->GetValue(lcLadderID));

						long nActionID = VarLong(pRow->GetValue(lcActionID));
						switch (nActionID) {
							case atEmailDocument:
								
								//call the function to email all documents associated with this step
								EmailDocuments(nStepID, nTypeID);

								//ask them if they would like to mark the step complete since this is a manual step
								if (IDYES == MsgBox(MB_YESNO, "Would you like to mark this step complete?")) {
									ExecuteParamSql("UPDATE ClientImplementationStepsT SET Done = 1, Active = 0 WHERE ID = {INT}", 
									nStepID);									
									NXDATALIST2Lib::IRowSettingsPtr pNextRow;
									pNextRow = pRow->GetNextRow();
									if (pNextRow) {
										if (VarLong(pNextRow->GetValue(lcLadderID), -1) == nLadderID) {
											long nNextStepID = pNextRow->GetValue(lcStepID);
											CompleteImplementationStep(nStepID, nNextStepID);
											LoadLadderList();
										}
									}
								}
								
							break;

							case atCompleteEMR:
								{
									//flip to the EMR tab
									CNxTabView* pView = (CNxTabView *)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
									if (pView) {
										pView->SetActiveTab(PatientsModule::NexEMRTab);		
									}
								}
								
							break;

							case atSelectProcedures:
								{

									CImplementationStepCriteriaSelectionDlg dlg(lttProcedures, nStepID, this);
									long nResult = dlg.DoModal();
									
								}

							break;

							case atCustomizeTemplates:
								{
									CImplementationStepCriteriaSelectionDlg dlg(lttEMR, nStepID, this);
									long nResult = dlg.DoModal();
									
								}
							break;

							case atMergeDocument:
								{
									CString strTemplateName = VarString(pRow->GetValue(lcAction), "");
									long nResult = strTemplateName.Find(":");
									if (nResult > -1) {
										strTemplateName = strTemplateName.Right(strTemplateName.GetLength() - (nResult + 1));
										strTemplateName.TrimRight();
										strTemplateName.TrimLeft();
									}
									if (!strTemplateName.IsEmpty()) {
										
										MergeDocument(strTemplateName, nDocumentCategoryID);
									}
								}
							break;

							case atMergePacket:
								MergePacket(nStepID, nDocumentCategoryID);
							break;

							case atAptByCategory:
							case atAptByType:
								GetMainFrame()->FlipToModule(SCHEDULER_MODULE_NAME);
							break;

							case atCustom:
								// do nothing
							break;
						} //end action switch
						} //end scope 
					}//end col switch
			}//end parent row if
		}//end prow if
				

	}NxCatchAll("Error in CImplementationDlg::OnLeftClickClientImplementationLadders");
	
}

void CImplementationDlg::OnRButtonUpClientImplementationLadders(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	
	try {

		CMenu menPopup;
		menPopup.m_hMenu = CreatePopupMenu();

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			//make sure this is a step row
			if (pRow->GetParentRow()) {

				m_pLadderList->CurSel = pRow;

				menPopup.InsertMenu(1, MF_BYPOSITION, ID_INSERT_ABOVE, "Insert Step &Above");
				menPopup.InsertMenu(2, MF_BYPOSITION, ID_INSERT_BELOW, "Insert Step &Below");
				menPopup.InsertMenu(3, MF_BYPOSITION | MF_SEPARATOR);
				menPopup.InsertMenu(4, MF_BYPOSITION, ID_DELETE_STEP, "&Delete Step");
				

				CPoint pt(x,y);
				CWnd* pWnd = GetDlgItem(IDC_CLIENT_IMPLEMENTATION_LADDERS);
				if (pWnd != NULL)
				{	pWnd->ClientToScreen(&pt);
					menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
				}
			}
			
		}	


	}NxCatchAll("Error in CImplementationDlg::OnRButtonUpClientImplementationLadders");

}

void CImplementationDlg::OnRowExpandedClientImplementationLadders(LPDISPATCH lpRow) 
{
	try { 

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
	
		if (pRow) {

			//check to make sure it is a parent row
			NXDATALIST2Lib::IRowSettingsPtr pParentRow = pRow->GetParentRow();

			if (pParentRow == NULL) {
			
				//good, its a parent
				m_dwExpandedLadders.Add(VarLong(pRow->GetValue(lcLadderID)));
			}
		}
	}NxCatchAll("Error in CImplementationDlg::OnRowExpandedClientImplementationLadders");
	
}

void CImplementationDlg::OnRowCollapsedClientImplementationLadders(LPDISPATCH lpRow) 
{
	try { 

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
	
		if (pRow) {

			//check to make sure it is a parent row
			NXDATALIST2Lib::IRowSettingsPtr pParentRow = pRow->GetParentRow();

			if (pParentRow == NULL) {

				long nLadderID = VarLong(pRow->GetValue(lcLadderID));

				for (int i = 0; i < m_dwExpandedLadders.GetSize(); i++) {

					if (((long)m_dwExpandedLadders.GetAt(i)) == nLadderID) {
						m_dwExpandedLadders.RemoveAt(i);
					}
				}
			}
		}
	}NxCatchAll("Error in CImplementationDlg::OnRowCollapsedClientImplementationLadders");
	
}

void CImplementationDlg::OnLadderSetup() 
{
	CConfigureImplementationLadderDlg dlg(this);
	dlg.DoModal();
	
}


void CImplementationDlg::AddLadderToClient(long nLadderID)  
{
	try {

		CString strSql;
		strSql = BeginSqlBatch();

		AddStatementToSqlBatch(strSql, "INSERT INTO ClientImplementationLadderT (ID, ClientID, MasterLadderTemplateID, Name, Active, TypeID, Done) "
			 " SELECT {LadderID}, %li, ID, Name, 1, TypeID, 0 FROM ImplementationLadderTemplateT WHERE ID = %li",
			 m_nPatientID, nLadderID);

		ADODB::_RecordsetPtr rsSteps = CreateParamRecordset("SELECT ID, ActionID FROM ImplementationStepTemplatesT WHERE LadderTemplateID = {INT}", nLadderID);
		
		long nCount = 0;
		while (!rsSteps->eof) {

			
			long nStepTemplateID = AdoFldLong(rsSteps, "ID");
			long nActionID = AdoFldLong(rsSteps, "ActionID");
			
			AddStatementToSqlBatch(strSql, "INSERT INTO ClientImplementationStepsT (ID, LadderID, ActionID, SortOrder, StepName, Note, CreateToDo, ToDOUserID, "
				" ToDoPriority, ToDoCategoryID, IsClientAssigned, IsWebVisible, DocumentCategoryID, AutoActivate, ActivateTimeInterval, ActivateTimeLength) "
				" SELECT ({BaseStepID} + %li), {LadderID}, ActionID, SortOrder, StepName, Note, CreateToDo, ToDoUserID, "
				" ToDoPriority, ToDoCategoryID, IsClientAssigned, IsWebVisible, DocumentCategoryID, AutoActivate, ActivateTimeInterval, ActivateTimeLength "
				" FROM ImplementationStepTemplatesT WHERE LadderTemplateID = %li AND ID = %li",
				nCount, nLadderID, nStepTemplateID);

			switch (nActionID) {

				case atEmailDocument:
				case atCompleteEMR:
				case atMergeDocument:
				case atMergePacket:
				case atAptByCategory:
				case atAptByType:

					AddStatementToSqlBatch(strSql, "INSERT INTO ClientImplementationStepCriteriaT (StepID, ActionItemID, ActionItemPath) "
						" SELECT ({BaseStepID} + %li), ActionItemID, ActionItemPath FROM ImplementationStepTemplatesCriteriaT WHERE StepTemplateID = %li",
						nCount, nStepTemplateID);

				break;

				case atSelectProcedures:
					AddStatementToSqlBatch(strSql, "INSERT INTO ClientImplementationStepCriteriaT(StepID, ActionItemID, ActionItemPath) "
						" SELECT ({BaseStepID} + %li), ID, '' FROM ImplementationProcedureT", nCount);
				break;
				case atCustomizeTemplates:
					AddStatementToSqlBatch(strSql, "INSERT INTO ClientImplementationStepCriteriaT(StepID, ActionItemID, ActionItemPath) "
						" SELECT ({BaseStepID} + %li), ID, '' FROM ImplementationEMRTemplatesT", nCount);					
				break;

				case atCustom:
					//insert nothing
				break;
			}

			nCount++;
			rsSteps->MoveNext();
		}			 

		long nNewLadderID = NewNumber("ClientImplementationLadderT", "ID");
		CString strNewLadderID;
		strNewLadderID.Format("%li", nNewLadderID);
		strSql.Replace("{LadderID}", strNewLadderID);

		long nBaseStepID = NewNumber("ClientImplementationStepsT", "ID");
		CString strBaseStepID;
		strBaseStepID.Format("%li", nBaseStepID);
		strSql.Replace("{BaseStepID}", strBaseStepID);

		ExecuteSqlBatch(strSql);

		//requery the datalist
		LoadLadderList();


	}NxCatchAll("Error in CImplementationDlg::AddLadderToClient");



}

void CImplementationDlg::OnAddImplementationLadder() 
{
	try {

		CImplementationLadderPickerDlg dlg(this);
		long nResult = dlg.DoModal();

		if (nResult == IDOK) {

			AddLadderToClient(dlg.m_nLadderID);
		}
		else {
			//they cancelled
		}		

	}NxCatchAll("Error in CImplementationDlg::OnAddImplementationLadder");
	
}


void CImplementationDlg::OnKillfocusEmrPointPerson() 
{
	try {

		Save(IDC_EMR_POINT_PERSON);

	}NxCatchAll("Error in CImplementationDlg::OnKillfocusEmrPointPerson() ");
	
}

void CImplementationDlg::OnKillfocusEmrPointEmail() 
{
	try {
	
		Save(IDC_EMR_POINT_EMAIL);
	
	}NxCatchAll("Error in CImplementationDlg::OnKillfocusEmrPointEmail");
	
}

void CImplementationDlg::OnKillfocusPmPointEmail() 
{
	try {
	
		Save(IDC_PM_POINT_EMAIL);
	
	}NxCatchAll("Error in CImplementationDlg::OnKillfocusPmPointEmail");
	
}

void CImplementationDlg::OnKillfocusPmPointPerson() 
{
	try {
	
		Save(IDC_PM_POINT_PERSON);
	
	}NxCatchAll("Error in CImplementationDlg::OnKillfocusPmPointPerson() ");
	
	
}

void CImplementationDlg::OnMoveStepUp() 
{
	try {

		//we just need to switch the two rows
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLadderList->CurSel;
		if (pRow) {
			NXDATALIST2Lib::IRowSettingsPtr pRowParent = pRow->GetParentRow();		

			if (pRowParent) {
				if (pRow != pRowParent->GetFirstChildRow()) {
					long nOrigOrderID = VarLong(pRow->GetValue(lcSortOrder));
					long nNewOrderID = nOrigOrderID - 1;

					long nLadderID = VarLong(pRowParent->GetValue(lcLadderID));

					ExecuteParamSql(" UPDATE ClientImplementationStepsT SET SortOrder = {INT} WHERE SortOrder = {INT} AND LadderID = {INT}; "
					" UPDATE ClientImplementationStepsT SET SortOrder = {INT} WHERE SortOrder = {INT} AND LadderID = {INT}; "
					" UPDATE ClientImplementationStepsT SET SortOrder = {INT} WHERE SortOrder = {INT} AND LadderID = {INT}; "
					,-1, nNewOrderID, nLadderID,
					nNewOrderID, nOrigOrderID, nLadderID,
					nOrigOrderID, -1, nLadderID);
											
					pRow->PutValue(lcSortOrder, nNewOrderID);

					pRow = pRow->GetPreviousRow();
					
					if (pRow) {
						pRow->PutValue(lcSortOrder, nOrigOrderID);
					}

					LoadLadderList();

					CheckButtonStatus();

				}
			}
		}

	}NxCatchAll("Error in CImplementationDlg::MoveStepUp");

	
}

void CImplementationDlg::CheckButtonStatus() {

	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLadderList->CurSel;
		if (pRow) {
			NXDATALIST2Lib::IRowSettingsPtr pRowParent = pRow->GetParentRow();

			if (pRowParent) {

				//its a child disable the delete button
				GetDlgItem(IDC_DELETE_IMPLEMENTATION_LADDER)->EnableWindow(FALSE);

				if (pRow == pRowParent->GetFirstChildRow()) {
					GetDlgItem(IDC_MOVE_STEP_UP)->EnableWindow(FALSE);
				}
				else {
					GetDlgItem(IDC_MOVE_STEP_UP)->EnableWindow(TRUE);
				}

				if (pRow == pRowParent->GetLastChildRow()) {
					GetDlgItem(IDC_MOVE_STEP_DOWN)->EnableWindow(FALSE);
				}
				else {
					GetDlgItem(IDC_MOVE_STEP_DOWN)->EnableWindow(TRUE);
				}
		
			}
			else {
				////this is a parent, disable the buttons
				GetDlgItem(IDC_MOVE_STEP_UP)->EnableWindow(FALSE);
				GetDlgItem(IDC_MOVE_STEP_DOWN)->EnableWindow(FALSE);

				//enable the delete button
				GetDlgItem(IDC_DELETE_IMPLEMENTATION_LADDER)->EnableWindow(TRUE);
			}
		}
		else {
			GetDlgItem(IDC_MOVE_STEP_UP)->EnableWindow(FALSE);
			GetDlgItem(IDC_MOVE_STEP_DOWN)->EnableWindow(FALSE);
		}

	}NxCatchAll("Error in CheckButtonStatus");
}

void CImplementationDlg::OnMoveStepDown() 
{
	try {

		//we just need to switch the two rows
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLadderList->CurSel;
		if (pRow) {
			NXDATALIST2Lib::IRowSettingsPtr pRowParent = pRow->GetParentRow();		

			if (pRowParent) {
				if (pRow != pRowParent->GetLastChildRow()) {
					long nOrigOrderID = VarLong(pRow->GetValue(lcSortOrder));
					long nNewOrderID = nOrigOrderID + 1;

					long nLadderID = VarLong(pRowParent->GetValue(lcLadderID));

					ExecuteParamSql(" UPDATE ClientImplementationStepsT SET SortOrder = {INT} WHERE SortOrder = {INT} AND LadderID = {INT}; "
					" UPDATE ClientImplementationStepsT SET SortOrder = {INT} WHERE SortOrder = {INT} AND LadderID = {INT}; "
					" UPDATE ClientImplementationStepsT SET SortOrder = {INT} WHERE SortOrder = {INT} AND LadderID = {INT}; "
					,-1, nNewOrderID, nLadderID,
					nNewOrderID, nOrigOrderID, nLadderID,
					nOrigOrderID, -1, nLadderID);
											
					pRow->PutValue(lcSortOrder, nNewOrderID);

					pRow = pRow->GetNextRow();
					
					if (pRow) {
						pRow->PutValue(lcSortOrder, nOrigOrderID);
					}

					LoadLadderList();

					CheckButtonStatus();

				}
			}
		}

	}NxCatchAll("Error in CImplementationDlg::MoveStepDown");
	
}

// (j.gruber 2008-02-19 15:24) - PLID 28070
void CImplementationDlg::OnKillfocusStatusNote() 
{
	try {

		Save(IDC_STATUS_NOTE);


	}NxCatchAll("Error in CImplementationDlg::OnKillfocusStatusNote");
	
}

void CImplementationDlg::OnDblClickCellClientImplementationLadders(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			NXDATALIST2Lib::IRowSettingsPtr pParentRow;
			pParentRow = pRow->GetParentRow();

			if (pParentRow) {
				long nStepID = VarLong(pRow->GetValue(lcStepID));
				long nLadderID = VarLong(pRow->GetValue(lcLadderID));
				CConfigureImplementationStepDlg dlg(nStepID, nLadderID, false, this);

				if (dlg.DoModal() == IDOK) {
					LoadLadderList();
				}
			}
		}
	}NxCatchAll("Error in CImplementationDlg::OnDblClickCellClientImplementationLadders");
	
}


void CImplementationDlg::ActivateStep(long nStepID) {

	try {

		ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT * FROM ClientImplementationStepsT WHERE ID = {INT}", nStepID);

		if (!rs->eof) {

			//see if we need to make a todo alarm
			if (AdoFldBool(rs, "CreateToDo")) {

				//yup, make a todo
				CString strToDoNote = "Implementation Step ToDo ";
				CString strToDoType;

				long nActionID = AdoFldLong(rs, "ActionID");

				switch (nActionID) {

					case atEmailDocument:
						strToDoNote += "Email Document(s) ";
						strToDoType = "Email";
					break;

					case atCompleteEMR:
						strToDoType += "Complete EMR ";
					break;

					case atSelectProcedures:
						strToDoType += "Select Procedures ";
					break;

					case atCustomizeTemplates:
						strToDoType += "Customize Templates ";
					break;

					case atMergeDocument:
						strToDoType += "Merge Document ";
					break;

					case atMergePacket:
						strToDoType += "Merge Packet ";
					break;

					case atAptByCategory:
						strToDoType += "Appt By Category ";
					break;

					case atAptByType:
						strToDoType += "Appt By Type ";
					break;
					
					case atCustom: 
						strToDoNote += "Custom Step ";
					break;
				}

				strToDoNote += AdoFldString(rs, "Note", "");

				COleDateTime dtActiveDate = AdoFldDateTime(rs, "ActiveDate", COleDateTime::GetCurrentTime());

				long nUserID = AdoFldLong(rs, "ToDoUserID");
				if (nUserID == -1) {
					//emr specialist
					ADODB::_RecordsetPtr rsUser = CreateParamRecordset("SELECT EMRSpecialistID FROM NxClientsT WHERE PersonID = {INT}", m_nPatientID);

					if (rsUser->eof) {
						//just use the current username
						nUserID = GetCurrentUserID();
					}
					else {
						nUserID = AdoFldLong(rsUser, "EmrSpecialistID", GetCurrentUserID());
					}
				}
				else if (nUserID == -2) {
					//install incident owner
					ADODB::_RecordsetPtr rsUser = CreateParamRecordset("SELECT UserID FROM IssueT WHERE CategoryID = 71 AND ClientID = {INT}", m_nPatientID);
					if (rsUser->eof) {
						nUserID = GetCurrentUserID();
					}
					else {
						nUserID = AdoFldLong(rsUser, "UserID", GetCurrentUserID());
					}
				}
								

				long nPriority = AdoFldLong(rs, "ToDoPriority", 3);
				long nCategory = AdoFldLong(rs, "ToDoCategoryID", -1);

				CString strSql = BeginSqlBatch();

				// (j.armen 2014-01-30 09:29) - PLID 18569 - Idenitate TodoList
				AddStatementToSqlBatch(strSql, "INSERT INTO ToDoList (Remind, Deadline, EnteredBy, PersonID, Notes, Priority, Task, LocationID, CategoryID, RegardingID, RegardingType) "
					" VALUES ('%s', '%s', %li, %li, '%s', %li, '%s', %li, %li, %li, %li)\r\n"
					"DECLARE @TaskID INT\r\n"
					"SET @TaskID = SCOPE_IDENTITY()\r\n",
					FormatDateTimeForSql(dtActiveDate), FormatDateTimeForSql(dtActiveDate),
					GetCurrentUserID(), m_nPatientID, _Q(strToDoNote), nPriority, _Q("Other"), 
					GetCurrentLocationID(), nCategory, nStepID, ttImplementationStep);

				// (c.haag 2008-06-11 11:45) - PLID 30321 - Also insert into TodoAssignToT
				AddStatementToSqlBatch(strSql, "INSERT INTO TodoAssignToT (TaskID, AssignTo) VALUES (@TaskID, %d)",
					nUserID);

				AddStatementToSqlBatch(strSql, "UPDATE ClientImplementationStepsT SET ToDoTaskID = @TaskID WHERE ID = %li", nStepID);

				ExecuteSqlBatch(strSql);
		
			}
		}


	}NxCatchAll("Error in CImplementationDlg::ActivateStep");
}


void CImplementationDlg::CompleteImplementationStep(long nStepID, long nNextStepID) {

	try {

		ADODB::_RecordsetPtr rsNextStep = CreateParamRecordset("SELECT * FROM ClientImplementationStepsT WHERE ID = {INT}", nNextStepID);

		if (! rsNextStep->eof) {

			//see if we are activating it now
			if (AdoFldBool(rsNextStep, "AutoActivate")) {
	
				_variant_t varActiveDate;
				varActiveDate = rsNextStep->Fields->Item["ActiveDate"]->Value;

				BOOL bActive = AdoFldBool(rsNextStep, "Active", FALSE);

				if (!bActive) {

			
					if (varActiveDate.vt == VT_NULL || varActiveDate.vt == VT_EMPTY) {
	
				
						//what is the timelength?
						long nTimeLength = AdoFldLong(rsNextStep, "ActivateTimeLength", 0);
						long nTimeInterval = AdoFldLong(rsNextStep, "ActivateTimeInterval");
						CString strInterval;
						COleDateTime dtToday;
						COleDateTime dtNow = COleDateTime::GetCurrentTime();
						dtToday.SetDate(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay());			

						if (nTimeInterval == tiImmediately) {
	
							strInterval.Format("'%s'", FormatDateTimeForSql(dtToday));
						}
						else {

							switch (nTimeInterval) {
	
								case tiDays:
									strInterval.Format("DateAdd(day, %li, '%s')",  nTimeLength, FormatDateTimeForSql(dtToday));
								break;
								case tiWeeks:
									strInterval.Format("DateAdd(week, %li, '%s')",  nTimeLength, FormatDateTimeForSql(dtToday));
								break;
								case tiMonths:
									strInterval.Format("DateAdd(month, %li, '%s')", nTimeLength, FormatDateTimeForSql(dtToday));
								break;
								case tiYears:
									strInterval.Format("DateAdd(year, %li, '%s')", nTimeLength, FormatDateTimeForSql(dtToday));
								break;
							}
						}

						ExecuteSql("UPDATE ClientImplementationStepsT SET ActiveDate = %s WHERE ID = %li", strInterval, nNextStepID);
					}
					ActivateStep(nNextStepID);
				}
				
			}
		}

	}NxCatchAll("CImplementationDlg::CompleteImplementationStep");
}

void CImplementationDlg::OnEditingFinishedClientImplementationLadders(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		if (bCommit) {

			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			NXDATALIST2Lib::IRowSettingsPtr pCheckRow = pRow->GetParentRow();

			if (pRow) {

				long nStepID = VarLong(pRow->GetValue(lcStepID), -1);
				long nLadderID = VarLong(pRow->GetValue(lcLadderID));

				switch (nCol) {

					case lcActive:

						//make sure this is a child row
						if (pCheckRow == NULL) {
							//its a parent row 
							return;
						}

						//only available for steps
						if (VarBool(varNewValue)) {
							ExecuteParamSql("UPDATE ClientImplementationStepsT SET Active = 1, ActiveDate = dbo.AsDateNoTime(getDate()) WHERE ID = {INT}", 
								nStepID);
							ActivateStep(nStepID);
						}
						else {
							ExecuteParamSql("UPDATE ClientImplementationStepsT SET Active = 0, ActiveDate =  NULL WHERE ID = {INT}", 
								nStepID);

							ADODB::_RecordsetPtr rsToDo = CreateParamRecordset("SELECT ToDoTaskID FROM ClientImplementationStepsT WHERE ID = {INT}", 
								nStepID);
							if (! rsToDo->eof) {
								long nToDoID = AdoFldLong(rsToDo, "ToDoTaskID", -1);
								if (nToDoID != -1) {
									if (IDYES == MsgBox(MB_YESNO, "Would you like to delete to corresponding todo item?")) {
										CString strSql = BeginSqlBatch();
										AddStatementToSqlBatch(strSql, "UPDATE ClientImplementationStepsT SET ToDoTaskID = NULL WHERE ID = %li", nStepID);
										// (c.haag 2008-06-11 11:04) - PLID 30328 - Also delete from TodoAssignToT
										// (c.haag 2008-07-10 15:36) - PLID 30674 - Ensure that the EMR todo table is also cleared
										AddStatementToSqlBatch(strSql, "DELETE FROM EMRTodosT WHERE TaskID = %li", nToDoID);
										AddStatementToSqlBatch(strSql, "DELETE FROM TodoAssignToT WHERE TaskID = %li", nToDoID);
										AddStatementToSqlBatch(strSql, "DELETE FROM ToDoList WHERE TaskID = %li", nToDoID);
										ExecuteSqlBatch(strSql);
									}
								}
							}
						}					
						
					break;

					// (j.gruber 2007-12-11 12:24) - PLID 28327 - be able to edit the ladder name
					case lcLadderName:

						//make sure this is a parent row
						if (pCheckRow == NULL) {
							
							ExecuteParamSql("UPDATE ClientImplementationLadderT SET Name = {STRING} WHERE ID = {INT}", 
								VarString(varNewValue, ""), nLadderID);
								
						}



					break;

					case lcDocDone:
						{

						//make sure this is a child row
						if (pCheckRow == NULL) {
							//its a parent row 
							return;
						}
						//only available for steps
						BOOL bIsClientAssigned = VarBool(pRow->GetValue(lcDocAssigned), FALSE);
						if (bIsClientAssigned) {
						
							ExecuteParamSql("UPDATE ClientImplementationStepsT SET ClientDone = {INT} WHERE ID = {INT}", 
								VarBool(varNewValue), nStepID);
						}
						else {
							MsgBox("This step cannot be assigned to a client, therefore you cannot mark its Client Completed Column");
						}
						}
					break;

					case lcDone:
						if (nStepID == -1) {
							//its a ladder
							ExecuteParamSql("UPDATE ClientImplementationLadderT SET Done = {INT} WHERE ID = {INT}", 
							VarBool(varNewValue), nLadderID);

							if (VarBool(varNewValue)) {
								ExecuteParamSql("UPDATE ClientImplementationStepsT SET Done = 1, Active = 0 WHERE LadderID = {INT}", 
								nLadderID);

								ExecuteParamSql("UPDATE ToDoList SET Done = GetDate() WHERE TaskID IN (SELECT ToDoTaskID FROM ClientImplementationStepsT WHERE LadderID = {INT}) AND ToDoList.Done IS NULL", nLadderID);
							}
							else {
								ExecuteParamSql("UPDATE ClientImplementationStepsT SET Done = 0 WHERE LadderID = {INT}", 
								nLadderID);
							}

						}
						else {
							if (VarBool(varNewValue)) {
								CString strSql = BeginSqlBatch();
								AddStatementToSqlBatch(strSql, "UPDATE ClientImplementationStepsT SET Done = 1, Active = 0 WHERE ID = %li ", 
									nStepID);

								AddStatementToSqlBatch(strSql, "UPDATE ToDoList SET Done = GetDate() WHERE TaskID = (SELECT ToDoTaskID FROM ClientImplementationStepsT WHERE ID = %li) AND ToDoList.Done IS NULL", nStepID);

								ExecuteSqlBatch(strSql);
							

								NXDATALIST2Lib::IRowSettingsPtr pNextRow;
								pNextRow = pRow->GetNextRow();
								if (pNextRow) {
									long nNextStepID = VarLong(pNextRow->GetValue(lcStepID), -1);
									CompleteImplementationStep(nStepID, nNextStepID);
								}
								else {
									//CompleteImplementationStep(nStepID, -1);
								}
							}
							else {
								ExecuteParamSql("UPDATE ClientImplementationStepsT SET Done = 0 WHERE ID = {INT}", 
								nStepID);
							}
					
						}

					break;
				}

				LoadLadderList();
			}
		}

	}NxCatchAll("Error in CImplementationDlg::OnEditingFinishedClientImplementationLadders");
	
}


void CImplementationDlg::OnInsertAbove() {

	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pLadderList->CurSel;

		if (pRow) {

			//get its step order

			long nStepOrder = VarLong(pRow->GetValue(lcSortOrder));
			long nPositionToInsertAt = nStepOrder;
			long nLadderID = VarLong(pRow->GetValue(lcLadderID));

			InsertStepAtPosition(nPositionToInsertAt, nLadderID);
		}

	}NxCatchAll("Error in CImplementationDlg::OnInsertAbove");


}


long CImplementationDlg::GetStepCountForLadder(long nLadderID) {

	try {

		NXDATALIST2Lib::IRowSettingsPtr pParentRow;
		pParentRow = m_pLadderList->FindByColumn(lcLadderID, nLadderID, NULL, FALSE);
		long nCount = 0;

		if (pParentRow) {

			//check to see if it is a parent
			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = pParentRow->GetParentRow();
			BOOL bContinue = TRUE;

			while (bContinue) {
				
				if (pRow == NULL) {
					//we found our parent row!
					bContinue = FALSE;
				}
				else {
					pParentRow = m_pLadderList->FindByColumn(lcLadderID, nLadderID, pRow, FALSE);

					if (pParentRow) {
						pRow = pParentRow->GetParentRow();
					}
					else {
						//we reached the end?
						ASSERT(FALSE);
						bContinue = FALSE;
					}

				}
			}

			if (pParentRow) {

				//loop through the parent and get the count
				pRow = pParentRow->GetFirstChildRow();
				NXDATALIST2Lib::IRowSettingsPtr pTempRow;
				pTempRow = pParentRow;
				nCount = 0;
				while (pRow != NULL && (pTempRow == pParentRow)) {
					nCount++;

					pRow = pRow->GetNextRow();
					if (pRow) {
						pTempRow = pRow->GetParentRow();
					}
				}

				return nCount;
			}
		}


			
		return 0;
	}NxCatchAll("Error in CImplementationDlg::GetStepCountForLadder");

	return -1;
}

void CImplementationDlg::InsertStepAtPosition(long nPos, long nLadderID) {

	try {

		CConfigureImplementationStepDlg dlg(-1, nLadderID, FALSE, this);
		long nResult = dlg.DoModal();

		if (nResult == IDOK) {

			//load the list to get the row into it
			LoadLadderList();

			//the step got inserted at the end, so we just need to rearrange all the steps 
			long nStepCount = GetStepCountForLadder(nLadderID);

			CString strSql = BeginSqlBatch();

			AddStatementToSqlBatch(strSql, "UPDATE ClientImplementationStepsT SET SortOrder = -1 WHERE SortOrder = %li AND LadderID = %li", nStepCount, nLadderID);

			for (int i = nStepCount-1; i >= nPos; i--) {

				AddStatementToSqlBatch(strSql, "UPDATE ClientImplementationStepsT SET SortOrder = %li WHERE SortOrder = %li AND LadderID = %li", i+1, i, nLadderID);
			}

			//now set the last one back
			AddStatementToSqlBatch(strSql, "UPDATE ClientImplementationStepsT SET SortOrder = %li WHERE SortOrder = -1 AND LadderID = %li", nPos, nLadderID);

			ExecuteSqlBatch(strSql);

			LoadLadderList();
		}


	}NxCatchAll("Error in CImplementationDlg::InsertStepAtPosition");


}

void CImplementationDlg::OnInsertBelow() {

	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pLadderList->CurSel;
		
		if (pRow) {

			//get its step order
			long nStepOrder = VarLong(pRow->GetValue(lcSortOrder));
			long nPositionToInsertAt(nStepOrder+1);
			long nLadderID = VarLong(pRow->GetValue(lcLadderID));

			InsertStepAtPosition(nPositionToInsertAt, nLadderID);
		}

	}NxCatchAll("Error in CImplementationDlg::OnInsertAbove");


}

void CImplementationDlg::OnDeleteStep() {

	try {

		//see if they are on a child ladder
		NXDATALIST2Lib::IRowSettingsPtr pStepRow;
		pStepRow = m_pLadderList->CurSel;

		if (pStepRow) {
			//make sure that it is indeed a child row
			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = pStepRow->GetParentRow();

			if (pRow) {

				//now warn them
				if (IDYES == MsgBox(MB_YESNO, "Are you sure you want to delete this step?")) {

					//they want to continue

					//get the stepID
					long nStepID = VarLong(pStepRow->GetValue(lcStepID));
					long nOrder = VarLong(pStepRow->GetValue(lcSortOrder));
					long nOrigLadderID = VarLong(pStepRow->GetValue(lcLadderID));
					CString strBatch = BeginSqlBatch();

					if (IDYES == MsgBox(MB_YESNO, "Would you like to delete any corresponding ToDo items associated with this ladder as well?")) {
						ADODB::_RecordsetPtr rsTodo = CreateParamRecordset("SELECT ToDoTaskID FROM ClientImplementationStepsT WHERE ID = {INT} AND ToDoTaskID IS NOT NULL", nStepID);
						if (!rsTodo->eof){ 
							long nTaskID = AdoFldLong(rsTodo, "ToDoTaskID", -1);
							AddStatementToSqlBatch(strBatch, "UPDATE ClientImplementationStepsT SET ToDoTaskID = NULL WHERE ID = %li", nStepID);
							// (c.haag 2008-06-11 11:04) - PLID 30328 - Also delete from TodoAssignToT
							// (c.haag 2008-07-10 15:36) - PLID 30674 - Ensure that the EMR todo table is also cleared
							AddStatementToSqlBatch(strBatch, "DELETE FROM EMRTodosT WHERE TaskID = %li", nTaskID);
							AddStatementToSqlBatch(strBatch, "DELETE FROM TodoAssignToT WHERE TaskID = %li", nTaskID);
							AddStatementToSqlBatch(strBatch, "DELETE FROM ToDoList WHERE TaskID = %li", nTaskID);
							rsTodo->MoveNext();
						}
					}

					AddStatementToSqlBatch(strBatch, "DELETE FROM ClientImplementationStepCriteriaT WHERE StepID = %li ", nStepID);

					AddStatementToSqlBatch(strBatch, "DELETE FROM ClientImplementationStepsT WHERE ID = %li", nStepID);


					//loop through the rows under this one and fix their order
					NXDATALIST2Lib::IRowSettingsPtr pRowNext;
					pRowNext = pStepRow->GetNextRow();

					long nStepLadderID;
					
					if (pRowNext) {
						
						nStepLadderID = VarLong(pRowNext->GetValue(lcLadderID));
					
					
						while (pRowNext != NULL && (nStepLadderID = nOrigLadderID)) {
							AddStatementToSqlBatch(strBatch, "UPDATE ClientImplementationStepsT SET SortOrder = %li WHERE ID = %li ",
								nOrder, VarLong(pRowNext->GetValue(lcStepID)));

							pRowNext->PutValue(lcSortOrder, nOrder);
	
							nOrder++;
	
							pRowNext = pRowNext->GetNextRow();
							if (pRowNext) {
								nStepLadderID = VarLong(pRowNext->GetValue(lcLadderID));
							}
						}
					}

					
					ExecuteSqlBatch(strBatch);

					LoadLadderList(FALSE);

					CheckButtonStatus();
				}
			}
		}



	}NxCatchAll("Error in CImplementationDlg::OnDeleteStep() ");


}



BOOL CImplementationDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (wParam) {

		case ID_INSERT_ABOVE:
			OnInsertAbove();
		break;

		case ID_INSERT_BELOW:
			OnInsertBelow();
		break;

		case ID_DELETE_STEP:
			OnDeleteStep();
		break;
	}
	
	return CPatientDialog::OnCommand(wParam, lParam);
}

void CImplementationDlg::OnDeleteImplementationLadder() 
{
	try {

		//see if they are on a parent ladder
		NXDATALIST2Lib::IRowSettingsPtr pParentRow;
		pParentRow = m_pLadderList->CurSel;

		if (pParentRow) {
			//make sure that it is indeed a parent row
			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = pParentRow->GetParentRow();

			if (pRow) {

				//our original wasn't actually a parent row, so let's make it that
				pParentRow = pRow;
			}

			//set the selection
			m_pLadderList->CurSel = pParentRow;

			//now warn them
			if (IDYES == MsgBox(MB_YESNO, "This will delete this entire ladder for the client, are you sure you wish to do this?")) {

				//they want to continue

				//get the ladderID
				long nLadderID = VarLong(pParentRow->GetValue(lcLadderID));
				CString strBatch = BeginSqlBatch();

				if (IDYES == MsgBox(MB_YESNO, "Would you like to delete any corresponding ToDo items associated with this ladder as well?")) {
					ADODB::_RecordsetPtr rsTodo = CreateParamRecordset("SELECT ID, ToDoTaskID FROM ClientImplementationStepsT WHERE LadderID = {INT} AND ToDoTaskID IS NOT NULL", nLadderID);
					while (!rsTodo->eof){ 
						long nTaskID = AdoFldLong(rsTodo, "ToDoTaskID", -1);
						long nStepID = AdoFldLong(rsTodo, "ID");
						AddStatementToSqlBatch(strBatch, "UPDATE ClientImplementationStepsT SET ToDoTaskID = NULL WHERE ID = %li ", nStepID);
						// (c.haag 2008-06-11 11:04) - PLID 30328 - Also delete from TodoAssignToT
						// (c.haag 2008-07-10 15:36) - PLID 30674 - Ensure that the EMR todo table is also cleared
						AddStatementToSqlBatch(strBatch, "DELETE FROM EMRTodosT WHERE TaskID = %li", nTaskID);
						AddStatementToSqlBatch(strBatch, "DELETE FROM TodoAssignToT WHERE TaskID = %li", nTaskID);
						AddStatementToSqlBatch(strBatch, "DELETE FROM ToDoList WHERE TaskID = %li", nTaskID);
						rsTodo->MoveNext();
					}
				}

				AddStatementToSqlBatch(strBatch, "DELETE FROM ClientImplementationStepCriteriaT WHERE StepID IN (SELECT ID FROM ClientImplementationStepsT WHERE LadderID = %li)", nLadderID);

				AddStatementToSqlBatch(strBatch, "DELETE FROM ClientImplementationStepsT WHERE LadderID = %li", nLadderID);

				AddStatementToSqlBatch(strBatch, "DELETE FROM ClientImplementationLadderT WHERE ID = %li", nLadderID);

				ExecuteSqlBatch(strBatch);

				LoadLadderList(FALSE);
			}
		}

		
	}NxCatchAll("Error in CImplementationDlg::OnDeleteImplementationLadder");

	
}



void CImplementationDlg::OnSelChangedClientImplementationLadders(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try  {

		CheckButtonStatus();


	}NxCatchAll("Error in CImplementationDlg:::OnSelChangedClientImplementationLadders");
	
}

BOOL CImplementationDlg::IsActionComplete(long nStepID, long nActionID) {

	try {
		BOOL bIsCompleted = FALSE;
		ADODB::_RecordsetPtr rsCheck;

		switch (nActionID) {

				case atEmailDocument:
					//this is a manually marked done step
				break;
				case atCompleteEMR:
				
					rsCheck = CreateParamRecordset("SELECT ID FROM EMRMasterT WHERE Status IN (1,2) AND PatientID = {INT} AND TemplateID IN (SELECT ActionItemID FROM ClientImplementationStepCriteriaT WHERE StepID = {INT})", m_nPatientID, nStepID);
					if (!rsCheck->eof) {
						bIsCompleted = TRUE;
					}
					rsCheck->Close();
					
				break;

				case atSelectProcedures:
					{
						BOOL bIsProcComplete = TRUE;
						rsCheck = CreateParamRecordset("SELECT ClientVisible, Complete FROM ClientImplementationStepCriteriaT WHERE StepID = {INT}", nStepID);
						while (! rsCheck->eof) {
							if (AdoFldBool(rsCheck, "ClientVisible", FALSE)) {
								if (!AdoFldBool(rsCheck, "Complete", FALSE)) {
									bIsProcComplete = FALSE;
								}
							}
							rsCheck->MoveNext();
						}
						if (bIsProcComplete) {
							bIsCompleted = TRUE;
						}
						rsCheck->Close();
					}
				break;

				case atCustomizeTemplates:
					{
						BOOL bIsTemplatesComplete = TRUE;
						rsCheck = CreateParamRecordset("SELECT ClientVisible, Complete FROM ClientImplementationStepCriteriaT WHERE StepID = {INT}", nStepID);
						while (! rsCheck->eof) {
							if (AdoFldBool(rsCheck, "ClientVisible", FALSE)) {
								if (!AdoFldBool(rsCheck, "Complete", FALSE)) {
									bIsTemplatesComplete = FALSE;
								}
							}
							rsCheck->MoveNext();
						}
						if (bIsTemplatesComplete) {
							bIsCompleted = TRUE;
						}
						rsCheck->Close();
					}
				break;

				case atMergeDocument:
					
					rsCheck = CreateParamRecordset("SELECT ActionItemPath FROM ClientImplementationStepCriteriaT WHERE StepID = {INT}", nStepID);
					if (!rsCheck->eof) {
						CString strPath = AdoFldString(rsCheck, "ActionItemPath", "");
						long nResult = strPath.ReverseFind('\\');
						CString strFileName = strPath.Right(strPath.GetLength() - (nResult + 1));
						strFileName.TrimRight(".dot");
						
						// (j.jones 2008-09-05 12:48) - PLID 30288 - supported MailSentNotesT
						rsCheck = CreateParamRecordset("SELECT MailSent.MailID "
							"FROM MailSent "
							"INNER JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
							"WHERE MailSent.PersonID = {INT} AND MailSentNotesT.Note = {STRING}", m_nPatientID, strFileName);
						if (! rsCheck->eof) {
							bIsCompleted = TRUE;
						}
					}
					rsCheck->Close();
					
				break;

				case atMergePacket:
					rsCheck = CreateParamRecordset("SELECT MailID FROM MailSent WHERE PersonID = {INT} AND MergedPacketID IN (SELECT ID FROM MergedPacketsT WHERE PacketID IN (SELECT ActionItemID FROM ClientImplementationStepCriteriaT WHERE StepID = {INT}))", m_nPatientID, nStepID);
					if (!rsCheck->eof) {
						bIsCompleted = TRUE;
					}
					rsCheck->Close();
				break;

				case atAptByCategory:
					
					rsCheck = CreateParamRecordset("SELECT ID FROM AppointmentsT WHERE Status <> 4 AND ShowState <> 3 AND AppointmentsT.Date <= GetDate() AND PatientID = {INT} AND AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category IN (SELECT ActionItemID FROM ClientImplementationStepCriteriaT WHERE StepID = {INT}))", m_nPatientID, nStepID);
					if (!rsCheck->eof) {
						bIsCompleted = TRUE;
					}
					rsCheck->Close();
				break;

				case atAptByType:
					rsCheck = CreateParamRecordset("SELECT ID FROM AppointmentsT WHERE Status <> 4 AND ShowState <> 3 AND AppointmentsT.Date <= GetDate() AND PatientID = {INT} AND AptTypeID IN (SELECT ActionItemID FROM ClientImplementationStepCriteriaT WHERE StepID = {INT})", m_nPatientID, nStepID);
					if (!rsCheck->eof) {
						bIsCompleted = TRUE;
					}
					rsCheck->Close();
				break;

				case atCustom:
					//we can't check anything here, so always return false
					bIsCompleted = FALSE;
				break;
			}

		return bIsCompleted;


	}NxCatchAll("Error in IsActionComplete");

	return FALSE;
}

void CImplementationDlg::OnAutoComplete() 
{
	try {

		ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT ID, ActionID, IsClientAssigned, ClientDone, Done FROM ClientImplementationStepsT WHERE LadderID IN (SELECT ID FROM ClientImplementationLadderT WHERE ClientID = {INT}) ORDER BY LadderID, SortOrder", m_nPatientID);
		ADODB::_RecordsetPtr rsCheck;

		CString strSql = BeginSqlBatch();

		while (! rs->eof) {

			//loop through each step and see if it is complete
			long nActionID = AdoFldLong(rs, "ActionID");
			long nStepID = AdoFldLong(rs, "ID");
			BOOL bIsClientAssigned = AdoFldBool(rs, "IsClientAssigned", FALSE);
			BOOL bIsClientDone = AdoFldBool(rs, "ClientDone", FALSE);
			BOOL bIsDone = AdoFldBool(rs, "Done", FALSE);
			BOOL bIsCompleted = FALSE;

			if (!bIsDone) {

				if (bIsClientAssigned) {
					if (bIsClientDone) {
						bIsCompleted = IsActionComplete(nStepID, nActionID);
					}
					else {
						//if the client isn't done, we can't mark it done
						bIsCompleted = FALSE;
					}
				}
				else {
					bIsCompleted = IsActionComplete(nStepID, nActionID);
				}
					

				if (bIsCompleted) {

					AddStatementToSqlBatch(strSql, "UPDATE ClientImplementationStepsT SET Done = 1, Active = 0 WHERE ID = %li", nStepID);
					AddStatementToSqlBatch(strSql, "UPDATE ToDoList SET Done = GetDate() WHERE TaskID = (SELECT ToDoTaskID FROM ClientImplementationStepsT WHERE ID = %li) AND Done IS NULL", nStepID);
				}
			}


			rs->MoveNext();
		}

		//now execute the batch
		ExecuteSqlBatch(strSql);

		LoadLadderList();


	}NxCatchAll("Error in CImplementationDlg::OnAutoComplete()");

}

// (j.gruber 2007-12-11 12:24) - PLID 28327 - be able to edit the ladder name
void CImplementationDlg::OnEditingStartingClientImplementationLadders(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {

		switch (nCol) {

			case lcLadderName:
				{
				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

				if (pRow) {

					//see if it is a parent
					NXDATALIST2Lib::IRowSettingsPtr pParentRow;
					pParentRow = pRow->GetParentRow();

					if (pParentRow) {
						//its a client row, we don't want none of that!
						*pbContinue = FALSE;
					}
				}
				}
			break;
		}

	}NxCatchAll("Error in CImplementationDlg::OnEditingStartingClientImplementationLadders");
	
}

// (j.gruber 2007-12-11 17:10) - PLID 28329 - made a go button for the website field
void CImplementationDlg::OnGoWebsite() 
{
	try  {

		CString strWebsite;
		GetDlgItemText(IDC_CLIENT_WEBSITE, strWebsite);

		//if it doesn't start with http:// try to fix it
		if (strWebsite.Find("http://") == -1) {
			//add it to the front, it might not fix all cases, but it should fix most
			strWebsite = "http://" + strWebsite;
		}

		// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
		ShellExecute(GetSafeHwnd(), NULL, "explorer.exe", strWebsite, NULL, SW_SHOW);

	}NxCatchAll("Error in OnGoWebsite");
	
}

// (j.gruber 2008-04-02 16:26) - PLID 28979 - ability to edit the specialist field
void CImplementationDlg::OnEditSpecialist() 
{
	try {

		
		// (z.manning 2008-09-25 17:44) - PLID 31348 - Check permission
		if(!CheckCurrentUserPermissions(bioEmrSpecialist, sptWrite)) {
			return;
		}

		_variant_t varValue;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSpecialistList->CurSel;
		if (pRow) {
			varValue = pRow->GetValue(0);
		}

		_variant_t varSecValue;
		pRow = m_pSecSpecList->CurSel;
		if (pRow) {
			varSecValue = pRow->GetValue(0);
		}

		CImplementationEditSpecialistDlg dlg(this);

		dlg.DoModal();

		m_pSpecialistList->Requery();
		m_pSecSpecList->Requery();

		
		pRow = m_pSpecialistList->GetNewRow();
		pRow->PutValue(spcID, (long) -1);
		pRow->PutValue(spcName, _variant_t("<No Specialist>"));
		m_pSpecialistList->AddRowSorted(pRow, NULL);

		pRow = m_pSecSpecList->GetNewRow();
		pRow->PutValue(spcID, (long) -1);
		pRow->PutValue(spcName, _variant_t("<No Specialist>"));
		m_pSecSpecList->AddRowSorted(pRow, NULL);

		pRow = m_pSpecialistList->SetSelByColumn(0, varValue);

		if (pRow == NULL) {
			m_pSpecialistList->SetSelByColumn(0, (long)-1);
		}

		pRow = m_pSecSpecList->SetSelByColumn(0, varSecValue);
		
		if (pRow == NULL) {
			m_pSecSpecList->SetSelByColumn(0, (long)-1);
		}

	}NxCatchAll("Error in OnEditSpecialist");
	
}

// (j.gruber 2008-04-02 13:13) - PLID 29443 - added ability to edit statuses
void CImplementationDlg::OnEditStatus() 
{
	try {
		_variant_t varValue;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pStatusList->CurSel;
		if (pRow) {
			varValue = pRow->GetValue(0);
		}

		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 19, m_pStatusList, "Edit EMR Statuses").DoModal();

		pRow = m_pStatusList->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _variant_t("<No Status>"));
		m_pStatusList->AddRowSorted(pRow, NULL);

		pRow = m_pStatusList->SetSelByColumn(0, varValue);
		
		if (pRow == NULL) {
			m_pStatusList->SetSelByColumn(0, (long)-1);
		}

		GetDlgItem(IDC_EMR_STATUS_LIST)->SetFocus();

	}NxCatchAll("Error in OnEditStatus");
	
}

// (j.gruber 2008-05-22 11:42) - PLID 30144
void CImplementationDlg::OnSelChosenEmrTypeList(LPDISPATCH lpRow) 
{
	try  {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		
		CString strTypeID;
		CString strType;
		if (pRow) {

			long nTypeID = VarLong(pRow->GetValue(tlcID), -1);
			strType = VarString(pRow->GetValue(tlcName), "");

			if (nTypeID == -1) {
				strTypeID = "NULL";
			}
			else {
				strTypeID.Format("%li", nTypeID);
			}
		}
		else {
			strTypeID = "NULL";
		}

		ExecuteSql("UPDATE NxClientsT SET EMRTypeID = %s WHERE PersonID = %li", 
			strTypeID, m_nPatientID);
	
		// (r.gonet 06/09/2010) - PLID 38871 - Audit changes in the Emr Type list.
		if(m_strOldEmrTypeID != strTypeID) {
			AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), BeginNewAuditEvent(), aeiImpEmrType, m_nPatientID, m_strOldEmrType, strType, aepMedium);
			m_strOldEmrTypeID = strTypeID;
			m_strOldEmrType = strType;
		}
		
	}NxCatchAll("Error in CImplementationDlg::OnSelChosenEmrTypeList");
	
}

void CImplementationDlg::OnSelChangingEmrTypeList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("CImplementationDlg::OnSelChangingEmrTypeList");
	
}

void CImplementationDlg::OnEditEmrTypeList() 
{
	// (j.gruber 2008-05-22 11:58) - PLID 30146
	try {

		_variant_t varValue;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTypeList->CurSel;
		if (pRow) {
			varValue = pRow->GetValue(0);
		}

		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 62, m_pTypeList, "Edit EMR Types").DoModal();

		pRow = m_pTypeList->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _variant_t("<No Type>"));
		m_pTypeList->AddRowSorted(pRow, NULL);

		pRow = m_pTypeList->SetSelByColumn(0, varValue);
		
		if (pRow == NULL) {
			m_pTypeList->SetSelByColumn(0, (long)-1);
		}

		GetDlgItem(IDC_EMR_TYPE_LIST)->SetFocus();

	}NxCatchAll("Error in OnEditTypes");
	
}

void CImplementationDlg::OnKillFocusFollowupDate() 
{
	try {

		if((m_bFollowupSet && m_pFollowupDate->GetStatus() == 3) || 
			((long)m_pFollowupDate->GetDateTime() != (long)m_dtFollowup.m_dt) 
			|| (!m_bFollowupSet && m_pFollowupDate->GetStatus() == 1)) {
				Save(IDC_FOLLOWUP_DATE);
		}

	}NxCatchAll("Error in OnKillFocusFollowupDate");
	
}

// (r.gonet 06/21/2010) - PLID 38871 - Get the EMR specialist id assigned to this client
long CImplementationDlg::GetEmrSpecialistID()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSpecialistList->CurSel;
	if(pRow) {
		return VarLong(pRow->GetValue(spcID), -1);
	} else {
		return -1;
	}
}

// (r.gonet 06/21/2010) - PLID 38871 - Get the EMR specialist assigned to this client
CString CImplementationDlg::GetEmrSpecialist()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSpecialistList->CurSel;
	if(pRow) {
		return VarString(pRow->GetValue(spcName), "");
	} else {
		return "";
	}
}

// (r.gonet 06/21/2010) - PLID 38871 - Get the EMR status ID of this client
long CImplementationDlg::GetEmrStatusID()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pStatusList->CurSel;
	if(pRow) {
		return VarLong(pRow->GetValue(stcID), -1);
	} else {
		return -1;
	}
}

// (r.gonet 06/21/2010) - PLID 38871 - Get the EMR status of this client
CString CImplementationDlg::GetEmrStatus()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pStatusList->CurSel;
	if(pRow) {
		return VarString(pRow->GetValue(stcName), "");
	} else {
		return "";
	}
}

// (r.gonet 06/21/2010) - PLID 38871 - Get the EMR type id of this client
long CImplementationDlg::GetEmrTypeID()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTypeList->CurSel;
	if(pRow) {
		return VarLong(pRow->GetValue(tlcID), -1);
	} else {
		return -1;
	}
}

// (r.gonet 06/21/2010) - PLID 38871 - Get the EMR type of this client
CString CImplementationDlg::GetEmrType()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTypeList->CurSel;
	if(pRow) {
		return VarString(pRow->GetValue(tlcName), "");
	} else {
		return "";
	}
}

// (j.gruber 2010-07-29 15:16) - PLID 39878
void CImplementationDlg::SelChangingSecEmrSpecialistList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-07-29 15:16) - PLID 39878
void CImplementationDlg::SelChosenSecEmrSpecialistList(LPDISPATCH lpRow)
{
	try  {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		
		CString strSpecialistID;
		CString strSpecialistName;
		if (pRow) {

			long nSpecialistID = VarLong(pRow->GetValue(spcID), -1);
			strSpecialistName = VarString(pRow->GetValue(spcName), "");

			if (nSpecialistID == -1) {
				strSpecialistID = "NULL";
			}
			else {
				strSpecialistID.Format("%li", nSpecialistID);
			}
		}
		else {
			strSpecialistID = "NULL";
		}

		ExecuteSql("UPDATE NxClientsT SET SecondaryEMRSpecialistID = %s WHERE PersonID = %li", 
			strSpecialistID, m_nPatientID);
	
		// (r.gonet 06/09/2010) - PLID 38871 - Audit changes in the Emr Specialist list
		if(m_strOldSecEmrSpecID != strSpecialistID) {
			AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), BeginNewAuditEvent(), aeiImpSecEmrSpecialist, m_nPatientID, m_strOldSecEmrSpec, strSpecialistName, aepMedium);
			m_strOldSecEmrSpecID = strSpecialistID;
			m_strOldSecEmrSpec = strSpecialistName;
		}
	
	}NxCatchAll(__FUNCTION__);
}
// (j.gruber 2010-07-29 15:16) - PLID 39878
void CImplementationDlg::TrySetSelFinishedSecEmrSpecialistList(long nRowEnum, long nFlags)
{
	try {
		if(nFlags == NXDATALIST2Lib::dlTrySetSelFinishedFailure) {
			//this really shouldn't happen since we have no inactives, everything should be accounted for
			ASSERT(FALSE);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-07-29 15:16) - PLID 39878
long CImplementationDlg::GetEmrSecSpecialistID()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSecSpecList->CurSel;
	if(pRow) {
		return VarLong(pRow->GetValue(spcID), -1);
	} else {
		return -1;
	}
}


// (j.gruber 2010-07-29 15:16) - PLID 39878
CString CImplementationDlg::GetEmrSecSpecialist()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSecSpecList->CurSel;
	if(pRow) {
		return VarString(pRow->GetValue(spcName), "");
	} else {
		return "";
	}
}