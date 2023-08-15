#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QMap>
#include <QMetaEnum>
#include <QObject>

namespace AppSettings {
Q_NAMESPACE

enum class SettingsKeys {
    Last_Project_Path,
    Platform_Path,
    Font_Family,
    Font_Size,
};
Q_ENUM_NS(SettingsKeys)

static const QMap<QString, QString> ProjectTypes{
    {"analytic_continuum", "Analytic Continuum"},
    {"binding_cells", "Binding Cells"},
    {"cell_division", "Cell Division"},
    {"diffusion", "Diffusion"},
    {"epidemiology", "Epidemiology"},
    {"flocking", "Flocking"},
    {"gene_regulation", "Gene Regulation"},
    {"makefile_project", "Makefile Project"},
    {"monolayre_growth", "Monolayer Growth"},
    {"multiple_simulations", "Multiple Simulations"},
    {"parameters", "Parameters"},
    {"pyramidal_cell", "Pyramidal Cell"},
    {"sbml_integration", "SBML Integration"},
    {"soma_clustering", "Soma Clustering"},
    {"tumor_concept", "Tumor Concept"},
};

template<class EnumClass>
static QString enumToString(const EnumClass &key)
{
    const auto metaEnum = QMetaEnum::fromType<EnumClass>();
    return metaEnum.valueToKey(static_cast<int>(key));
}

}; // namespace AppSettings

#endif // APPSETTINGS_H
