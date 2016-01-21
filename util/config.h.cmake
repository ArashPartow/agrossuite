#ifndef __AGROS_CONFIG_H_
#define __AGROS_CONFIG_H_

const int VERSION_MAJOR = ${VERSION_MAJOR};
const int VERSION_MINOR = ${VERSION_MINOR};
const int VERSION_SUB = ${VERSION_SUB};
const int VERSION_YEAR = ${VERSION_YEAR};
const int VERSION_MONTH = ${VERSION_MONTH}.0;
const int VERSION_DAY = ${VERSION_DAY}.0;

#cmakedefine WITH_OPENMP
#cmakedefine DEAL_II_WITH_UMFPACK

#endif
