# divseek-search

This module uses the [Chado Custom Search API](https://github.com/uofs-pulse-binfo/chado_custom_search) to create fast search/browse functionality. It was originally designed for the DivSeek Canada Tripal portal but can be used with any Tripal site.

## Dependencies
1. Tripal 3.x Drupal 7.x site
2. [Chado Custom Search API](https://github.com/uofs-pulse-binfo/chado_custom_search)

## Search/Browse Pages

### Germplasm Accessions
We load germplasm using the [Germplasm Accession Importer](https://github.com/uofs-pulse-binfo/tripal_germplasm_importer). This provides a standardized way of importing accession information including attribution such as the suppling institute and metadata such as common names, accessions, germplasm status codes and free-text pedigrees.

If you have existing data or want to use your own data loading method, the search expects the following chado stock properties: biological status of accession (CO_010:0000022), Breeding institute name (CO_020:0000097), country of origin (CO_010:0000010), pedigree information (CO_010:0000052). These terms are all part of the [Germplasm Crop Ontology](https://www.cropontology.org/ontology/CO_010/Germplasm).
