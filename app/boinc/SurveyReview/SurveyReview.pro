include(../../../orsa.pri)

TEMPLATE = subdirs

SUBDIRS += SurveyReview.app.pro 

unix:!macx {
	SUBDIRS += SurveyReviewWorkGenerator.app.pro
	SUBDIRS += SurveyReviewValidator.app.pro
	SUBDIRS += SurveyReviewAssimilator.app.pro
	SUBDIRS += DetectionEfficiency.app.pro
	SUBDIRS += DetectionEfficiencyFit.app.pro
	SUBDIRS += DetectionEfficiencyPlot.app.pro
	SUBDIRS += GenMake.app.pro
	SUBDIRS += ExtractObservations.app.pro
}
